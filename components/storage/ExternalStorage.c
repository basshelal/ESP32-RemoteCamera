#include "ExternalStorage.h"
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <diskio_impl.h>
#include <dirent.h>
#include <stdatomic.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "TaskWatcher.h"
#include "Logger.h"
#include "StorageCommon.h"

#define SD_CARD_PATH "/sd"
#define AUTOMOUNT_TASK_NAME "automountTask"
#define AUTOMOUNT_TASK_STACK_SIZE 2400
#define AUTOMOUNT_TASK_STACK_MIN AUTOMOUNT_TASK_STACK_SIZE * 0.05
#define AUTOMOUNT_TASK_PRIORITY 2U
#define AUTOMOUNT_TASK_POLL_MILLIS 1000
#define AUTOMOUNT_KILL_RETRY_ATTEMPTS 5

#define getPath(name, path) \
char name[EXTERNAL_STORAGE_MAX_PATH_LENGTH];   \
getPrefixedPath(name, SD_CARD_PATH, path)

private struct {
    bool isInitialized;
    sdmmc_host_t *sdmmcHost;
    atomic_bool isMounted;
    bool hasSDCard;
    sdmmc_card_t *card;
    struct {
        TaskHandle_t handle;
        bool isRunning;
    } task;
} this;

private Error externalStorage_mountSDCard() {
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (isMounted) return ERROR_NONE;
    sdspi_device_config_t deviceConfig = SDSPI_DEVICE_CONFIG_DEFAULT();
    deviceConfig.gpio_cs = 15;
    deviceConfig.host_id = this.sdmmcHost->slot;

    esp_vfs_fat_sdmmc_mount_config_t mountConfig = {
            .format_if_mount_failed = false,
            .max_files = 8,
    };
    esp_err_t err = esp_vfs_fat_sdspi_mount(
            /*base_path=*/SD_CARD_PATH,
            /*host_config_input=*/this.sdmmcHost,
            /*slot_config=*/&deviceConfig,
            /*mount_config=*/&mountConfig,
            /*out_card=*/&this.card);

    if (err == ESP_OK) {
        atomic_store(&this.isMounted, true);
    } else {
        throwESPError(esp_vfs_fat_sdspi_mount, err);
    }
    return ERROR_NONE;
}

private void externalStorage_autoMountTaskFunction(void *arg);

private void externalStorage_startAutoMountTask() {
    this.task.isRunning = true;
    xTaskCreate(
            /*pvTaskCode=*/externalStorage_autoMountTaskFunction,
            /*pcName=*/AUTOMOUNT_TASK_NAME,
            /*usStackDepth=*/AUTOMOUNT_TASK_STACK_SIZE,
            /*pvParameters=*/&this,
            /*uxPriority=*/AUTOMOUNT_TASK_PRIORITY,
            /*pxCreatedTask=*/this.task.handle
    );
}

private void externalStorage_autoMountTaskFunction(void *arg) {
    taskWatcher_notifyTaskStarted(AUTOMOUNT_TASK_NAME);
    typeof(this) *thisPtr = (typeof(this) *) arg;
    uint remaining = 0;
    while (thisPtr->task.isRunning) {
        remaining = uxTaskGetStackHighWaterMark(thisPtr->task.handle);
        if (remaining < AUTOMOUNT_TASK_STACK_MIN) {
            ERROR("Automount task ran out of stack, bytes remaining: %u", remaining);
            this.task.isRunning = false;
            break;
        }
        const bool oldHasSDCard = thisPtr->hasSDCard;
        const bool newHasSDCard = externalStorage_hasSDCard();
        const atomic_bool isMounted = atomic_load(&thisPtr->isMounted);
        if (oldHasSDCard != newHasSDCard) {
            thisPtr->hasSDCard = newHasSDCard;
            INFO("SD CARD: %s", boolToString(newHasSDCard));
            if (newHasSDCard && !isMounted) {
                externalStorage_mountSDCard();
            } else if (!newHasSDCard && isMounted) {
                externalStorage_unmountSDCard();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(AUTOMOUNT_TASK_POLL_MILLIS));
    }
    taskWatcher_notifyTaskStopped(
            /*taskName=*/AUTOMOUNT_TASK_NAME,
            /*shouldRestart=*/true,
            /*remainingStackBytes=*/remaining);
    vTaskDelete(thisPtr->task.handle);
    thisPtr->task.handle = NULL;
}

/*============================= Public API ==================================*/

public Error externalStorage_init(ExternalStorageOptions *externalStorageOptions) {
    if (this.isInitialized) {
        WARN("ExternalStorage has already been initialized");
        return ERROR_NONE;
    }
    requireArgNotNull(externalStorageOptions);
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    this.sdmmcHost = new(sdmmc_host_t);
    memcpy(this.sdmmcHost, &host, sizeof(sdmmc_host_t));

    spi_bus_config_t spiBusConfig = {
            .mosi_io_num = 12,
            .miso_io_num = 13,
            .sclk_io_num = 14,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4096,
    };
    esp_err_t err = spi_bus_initialize(this.sdmmcHost->slot, &spiBusConfig, SPI_DMA_CH_AUTO);

    gpio_config_t gpioConfig = {
            .pin_bit_mask = GPIO_SEL_36,
            .mode = GPIO_MODE_INPUT,
            .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_reset_pin(GPIO_NUM_36);
    gpio_config(&gpioConfig);

    this.hasSDCard = externalStorage_hasSDCard();
    atomic_init(&this.isMounted, false);
    if (this.hasSDCard) {
        throwIfError(externalStorage_mountSDCard(), "externalStorage_mountSDCard() failed");
    }

    if (externalStorageOptions->startAutoMountTask) {
        TaskInfo taskInfo = {
                .name = AUTOMOUNT_TASK_NAME,
                .stackBytes = AUTOMOUNT_TASK_STACK_SIZE,
                .startFunction = externalStorage_startAutoMountTask
        };
        taskWatcher_registerTask(&taskInfo);
        externalStorage_startAutoMountTask();
    }

    this.isInitialized = true;
    return ERROR_NONE;
}

public Error externalStorage_destroy() {
    if (!this.isInitialized) {
        throw(ERROR_NOT_INITIALIZED, "External Storage was not initialized, cannot destroy");
    }
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (isMounted) {
        throw(ERROR_ILLEGAL_STATE, "Cannot destroy External Storage while mounted, unmount!");
    }
    if (this.task.isRunning) {
        this.task.isRunning = false;
        int retries = 0;
        while (this.task.handle != NULL && retries < AUTOMOUNT_KILL_RETRY_ATTEMPTS) { // block until task is killed
            vTaskDelay(pdMS_TO_TICKS(AUTOMOUNT_TASK_POLL_MILLIS));
            retries++;
        }
        if (retries >= AUTOMOUNT_KILL_RETRY_ATTEMPTS) {
            throw(ERROR_ILLEGAL_STATE, "Couldn't kill automount task");
        }
    }
    spi_bus_free(this.sdmmcHost->slot);
    this.isInitialized = false;
    return ERROR_NONE;
}

public Error externalStorage_unmountSDCard() {
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (!isMounted) {
        return ERROR_NONE;
    }
    esp_err_t err = esp_vfs_fat_sdcard_unmount(SD_CARD_PATH, this.card);
    INFO("unmount() returned: %i: %s", err, esp_err_to_name(err));
    if (err == ESP_OK) {
        atomic_store(&this.isMounted, false);
    }
    return ERROR_NONE;
}

public bool externalStorage_hasSDCard() {
    // card detect pin is 1 when card is in and 0 when not in
    const bool cardDetectPinLevel = gpio_get_level(GPIO_NUM_36);
    return cardDetectPinLevel;
}

public Error externalStorage_getStorageInfo(StorageInfo *storageInfo) {
    requireArgNotNull(storageInfo);
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (!this.card || !this.hasSDCard || !isMounted) {
        throw(ERROR_NOT_FOUND, "Cannot query storage info, card is not present and/or not mounted");
    }
    DWORD freeClusters;
    FATFS *fatfs = NULL;
    FRESULT fresult = f_getfree(SD_CARD_PATH, &freeClusters, &fatfs);
    if (fresult != FR_OK) {
        throw(ERROR_LIBRARY_FAILURE, "f_getfree() returned %i: %s", fresult, fresultToString(fresult));
    }
    requireNotNull(fatfs, ERROR_LIBRARY_FAILURE,
                   "f_getfree() did not modify FATFS pointer which is NULL");
    uint64_t freeBytes = ((uint64_t) freeClusters) * ((uint64_t) fatfs->csize) * ((uint64_t) (fatfs->ssize));
    uint64_t totalBytes = ((uint64_t) fatfs->n_fatent - 2) * ((uint64_t) fatfs->csize) * ((uint64_t) (fatfs->ssize));
    uint64_t usedBytes = totalBytes - freeBytes;
    storageInfo->mountPoint = SD_CARD_PATH;
    storageInfo->totalBytes = totalBytes;
    storageInfo->usedBytes = usedBytes;
    storageInfo->freeBytes = freeBytes;
    return ERROR_NONE;
}

/*============================= Directories =================================*/

public Error externalStorage_queryDirExists(const char *dirPath, bool *dirExists) {
    requireArgNotNull(dirPath);
    requireArgNotNull(dirExists);
    getPath(path, dirPath);

    return storage_queryDirExists(path, dirExists);
}

// TODO: 11-Sep-2022 @basshelal: This will probably be deleted!
public Error externalStorage_queryDirInfo(const char *dirPath, DirInfo *dirInfo) {
    requireArgNotNull(dirPath);
    requireArgNotNull(dirInfo);
    getPath(path, dirPath);

    return storage_queryDirInfo(path, dirInfo);
}

public Error externalStorage_createDir(const char *dirPath) {
    requireArgNotNull(dirPath);
    getPath(path, dirPath);

    return storage_createDir(path);
}

public Error externalStorage_readDir(const char *dirPath, char **dirEntries, size_t *entryCount) {
    requireArgNotNull(dirPath);
    requireArgNotNull(entryCount);
    getPath(path, dirPath);

    return storage_readDir(path, dirEntries, entryCount);
}

public Error externalStorage_moveDir(const char *dirPath, const char *newDirPath) {
    requireArgNotNull(dirPath);
    requireArgNotNull(newDirPath);
    getPath(oldPath, dirPath);
    getPath(newPath, newDirPath);

    return storage_moveDir(oldPath, newPath);
}

public Error externalStorage_deleteDir(const char *dirPath) {
    requireArgNotNull(dirPath);
    getPath(path, dirPath);

    return storage_deleteDir(path);
}

/*============================= Files =======================================*/

public Error externalStorage_queryFileExists(const char *filePath, bool *fileExists) {
    requireArgNotNull(filePath);
    requireArgNotNull(fileExists);
    getPath(path, filePath);

    return storage_queryFileExists(path, fileExists);
}

public Error externalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo) {
    requireArgNotNull(filePath);
    requireArgNotNull(fileInfo);
    getPath(path, filePath);

    return storage_queryFileInfo(path, fileInfo);
}

public Error externalStorage_createFile(const char *filePath) {
    requireArgNotNull(filePath);
    getPath(path, filePath);

    return storage_createFile(path);
}

public Error externalStorage_openFile(const char *filePath, FILE **fileIn, const FileMode fileMode) {
    requireArgNotNull(filePath);
    requireArgNotNull(fileIn);
    getPath(path, filePath);

    return storage_openFile(path, fileIn, fileMode);
}

public Error externalStorage_closeFile(const FILE *fileIn) {
    requireArgNotNull(fileIn);

    return storage_closeFile(fileIn);
}

public Error externalStorage_readFile(const FILE *file, const size_t startPosition,
                                      void *bufferIn, const uint bufferLength, uint *bytesRead) {
    requireArgNotNull(file);
    requireArgNotNull(bufferIn);
    requireArgNotNull(bytesRead);

    return storage_readFile(file, startPosition, bufferIn, bufferLength, bytesRead);
}

public Error externalStorage_writeFile(const FILE *file, const size_t startPosition,
                                       const void *buffer, const uint bufferLength, uint *bytesWritten) {
    requireArgNotNull(file);
    requireArgNotNull(buffer);
    requireArgNotNull(bytesWritten);

    return storage_writeFile(file, startPosition, buffer, bufferLength, bytesWritten);
}

public Error externalStorage_moveFile(const char *filePath, const char *newFilePath) {
    requireArgNotNull(filePath);
    requireArgNotNull(newFilePath);

    getPath(oldPath, filePath);
    getPath(newPath, newFilePath);

    return storage_moveFile(oldPath, newPath);
}

public Error externalStorage_deleteFile(const char *filePath) {
    requireArgNotNull(filePath);

    getPath(path, filePath);

    return storage_deleteFile(path);
}