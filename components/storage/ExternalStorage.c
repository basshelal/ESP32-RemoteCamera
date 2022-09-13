#include "ExternalStorage.h"
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <diskio_impl.h>
#include <dirent.h>
#include <stdatomic.h>
#include <sys/stat.h>
#include <sys/errno.h>
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

#define requireParamNotNull(element, elementToString) \
requireNotNull(element, STORAGE_ERROR_INVALID_PARAMETER, elementToString" cannot be NULL")

#define requirePathLengthUnderLimit(path) \
size_t _length = strlen(path); \
require(_length <= EXTERNAL_STORAGE_MAX_PATH_LENGTH, STORAGE_ERROR_INVALID_LENGTH, \
        "path length cannot exceed limit of %u, was %u", EXTERNAL_STORAGE_MAX_PATH_LENGTH, _length)

#define getPrefixedPath(buffer, path) \
sprintf(buffer, "%s/%s", SD_CARD_PATH, path)

private StorageError externalStorage_mountSDCard() {
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (isMounted) return STORAGE_ERROR_NONE;
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
        throw(STORAGE_ERROR_GENERIC_FAILURE,
              "mount() returned: %i: %s", err, esp_err_to_name(err));
    }
    return STORAGE_ERROR_NONE;
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

private StorageError externalStorage_deleteDirAndContents(const char *dirPath) {
    List *stack = list_create();
    List *dirsToDelete = list_create();
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    struct dirent *entry;

    // This algorithm is essentially an iterative Depth First Search using a stack (List made to behave like a stack)
    //  we delete files as we find them, and add dirs in the order which we encounter them which will be from
    //  highest to lowest in the chain, meaning that list reversed will always be safe to delete because the dirs
    //  will be empty
    list_addItem(stack, dirPath); // begin with root dir
    while (!list_isEmpty(stack)) {
        const index_t stackLastIndex = list_getSize(stack) - 1;

        // all "processing" is done on the popped dir which is like the "current" dir
        const char *poppedDir = list_getItem(stack, stackLastIndex);
        list_removeItemIndexed(stack, stackLastIndex);
        list_addItem(dirsToDelete, poppedDir); // move from stack to dirsToDelete list

        const DIR *dir = opendir(poppedDir);
        if (dir == NULL) continue;

        while ((entry = readdir(dir))) {
            sprintf(pathBuffer, "%s/%s", poppedDir, entry->d_name);
            if (entry->d_type == DT_REG) { // file
                remove(pathBuffer); // delete files as we find them
            } else if (entry->d_type == DT_DIR) { // dir
                char *pathCopy = alloc(strlen(pathBuffer) + 1);
                strcpy(pathCopy, pathBuffer);
                list_addItem(stack, pathCopy); // add to stack to be processed on next loop
            }
        }
        closedir(dir);
    }
    // Reverse loop on dirsToDelete ensures that rmdir calls are always done on empty dirs
    for (int i = (int) list_getSize(dirsToDelete) - 1; i >= 0; i--) {
        const char *dirToDelete = list_getItem(dirsToDelete, i);
        if (dirToDelete == NULL) continue;
        if ((rmdir(dirToDelete) != 0)) {
            int err = errno;
            throw(STORAGE_ERROR_GENERIC_FAILURE, "rmdir() returned %i: %s", err, strerror(err));
        }
        if (dirToDelete != dirPath) { // all but root were allocated, so they need to be freed
            free(dirToDelete);
        }
    }
    list_destroy(stack);
    list_destroy(dirsToDelete);
    return STORAGE_ERROR_NONE;
}

/*============================= Public API ==================================*/

public StorageError externalStorage_init(ExternalStorageOptions *externalStorageOptions) {
    if (this.isInitialized) {
        WARN("ExternalStorage has already been initialized");
        return STORAGE_ERROR_NONE;
    }
    requireNotNull(externalStorageOptions, STORAGE_ERROR_INVALID_PARAMETER, "externalStorageOptions cannot be NULL");
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
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_destroy() {
    if (!this.isInitialized) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "External Storage was not initialized, cannot destroy");
    }
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (isMounted) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "Cannot destroy External Storage while mounted, unmount!");
    }
    if (this.task.isRunning) {
        this.task.isRunning = false;
        int retries = 0;
        while (this.task.handle != NULL && retries < AUTOMOUNT_KILL_RETRY_ATTEMPTS) { // block until task is killed
            vTaskDelay(pdMS_TO_TICKS(AUTOMOUNT_TASK_POLL_MILLIS));
            retries++;
        }
        if (retries >= AUTOMOUNT_KILL_RETRY_ATTEMPTS) {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "Couldn't kill automount task");
        }
    }
    spi_bus_free(this.sdmmcHost->slot);
    this.isInitialized = false;
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_unmountSDCard() {
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (!isMounted) {
        return STORAGE_ERROR_NONE;
    }
    esp_err_t err = esp_vfs_fat_sdcard_unmount(SD_CARD_PATH, this.card);
    INFO("unmount() returned: %i: %s", err, esp_err_to_name(err));
    if (err == ESP_OK) {
        atomic_store(&this.isMounted, false);
    }
    return STORAGE_ERROR_NONE;
}

public bool externalStorage_hasSDCard() {
    // card detect pin is 1 when card is in and 0 when not in
    const bool cardDetectPinLevel = gpio_get_level(GPIO_NUM_36);
    return cardDetectPinLevel;
}

public StorageError externalStorage_getStorageInfo(StorageInfo *storageInfo) {
    requireParamNotNull(storageInfo, "storageInfo");
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (!this.card || !this.hasSDCard || !isMounted) {
        throw(STORAGE_ERROR_PARTITION_NOT_FOUND, "Cannot query storage info, card is not present and/or not mounted");
    }
    DWORD freeClusters;
    FATFS *fatfs = NULL;
    FRESULT fresult = f_getfree(SD_CARD_PATH, &freeClusters, &fatfs);
    if (fresult != FR_OK) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "f_getfree() returned %i: %s", fresult, fresultToString(fresult));
    }
    requireNotNull(fatfs, STORAGE_ERROR_INVALID_PARAMETER,
                   "f_getfree() did not modify FATFS pointer which is NULL");
    uint64_t freeBytes = ((uint64_t) freeClusters) * ((uint64_t) fatfs->csize) * ((uint64_t) (fatfs->ssize));
    uint64_t totalBytes = ((uint64_t) fatfs->n_fatent - 2) * ((uint64_t) fatfs->csize) * ((uint64_t) (fatfs->ssize));
    uint64_t usedBytes = totalBytes - freeBytes;
    storageInfo->mountPoint = SD_CARD_PATH;
    storageInfo->totalBytes = totalBytes;
    storageInfo->usedBytes = usedBytes;
    storageInfo->freeBytes = freeBytes;
    return STORAGE_ERROR_NONE;
}

/*============================= Directories =================================*/

public StorageError externalStorage_queryDirExists(const char *dirPath, bool *dirExists) {
    requireParamNotNull(dirPath, "dirPath");
    requireParamNotNull(dirExists, "dirExists");
    requirePathLengthUnderLimit(dirPath);
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, dirPath);
    DIR *dir = opendir(pathBuffer);
    if (dir == NULL) {
        int err = errno;
        if (err == ENOENT) {
            *dirExists = false;
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "opendir() returned %i: %s", err, strerror(err));
        }
    } else {
        *dirExists = true;
        closedir(dir);
    }
    return STORAGE_ERROR_NONE;
}

// TODO: 11-Sep-2022 @basshelal: This will probably be deleted!
public StorageError externalStorage_queryDirInfo(const char *dirPath, DirInfo *dirInfo) {
    requireParamNotNull(dirPath, "dirPath");
    requireParamNotNull(dirInfo, "dirInfo");
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, dirPath);
    struct stat statResult;
    if (stat(pathBuffer, &statResult) != 0) {
        int err = errno;
        if (err == ENOENT || err == ENOTDIR) {
            throw(STORAGE_ERROR_NOT_FOUND, "dir not found: %s", dirPath);
        } else if (err != 0) {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "stat returned %i: %s", err, strerror(err));
        } else {
            // TODO: 04-Sep-2022 @basshelal: Implement or reconsider API
        }
    }
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_createDir(const char *dirPath) {
    requireParamNotNull(dirPath, "dirPath");
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, dirPath);

    if (mkdir(pathBuffer, ACCESSPERMS) != 0) {
        int err = errno;
        throw(STORAGE_ERROR_GENERIC_FAILURE, "mkdir returned %i: %s", err, strerror(err));
    }

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_readDir(const char *dirPath, char **dirEntries, size_t *entryCount) {
    requireParamNotNull(dirPath, "dirPath");
    requireParamNotNull(entryCount, "entryCount");

    bool fillEntries = true;
    if (dirEntries == NULL) { // user only wants count and not to fill array
        fillEntries = false;
    }

    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, dirPath);

    DIR *dir = opendir(pathBuffer);
    if (dir == NULL) {
        int err = errno;
        if (err == ENOENT) {
            *entryCount = 0;
            throw(STORAGE_ERROR_NOT_FOUND, "dir not found: %s", dirPath);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "opendir() returned %i: %s", err, strerror(err));
        }
    }
    size_t entries = 0;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (fillEntries) {
            dirEntries[entries] = alloc(sizeof(char) * strlen(entry->d_name));
            strcpy(dirEntries[entries], entry->d_name);
        }
        entries++;
    }
    *entryCount = entries;

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_moveDir(const char *dirPath, const char *newDirPath) {
    requireParamNotNull(dirPath, "dirPath");
    requireParamNotNull(newDirPath, "newDirPath");
    char oldPathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(oldPathBuffer, dirPath);
    char newPathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(newPathBuffer, newDirPath);
    if (rename(oldPathBuffer, newPathBuffer) != 0) {
        int err = errno;
        if (err == ENOENT) {
            throw(STORAGE_ERROR_NOT_FOUND, "dir(s) not found: %s -> %s", dirPath, newDirPath);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "rename() returned %i: %s", err, strerror(err));
        }
    }
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_deleteDir(const char *dirPath) {
    requireParamNotNull(dirPath, "dirPath");
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, dirPath);
    bool dirExists;
    externalStorage_queryDirExists(dirPath, &dirExists);
    if (!dirExists) {
        throw(STORAGE_ERROR_NOT_FOUND, "dir not found: %s", dirPath);
    }
    throwIfError(externalStorage_deleteDirAndContents(pathBuffer), "could not delete dir and/or contents");
    return STORAGE_ERROR_NONE;
}

/*============================= Files =======================================*/

// TODO: 11-Sep-2022 @basshelal: Most if not all of this can be shared with internal storage with minor changes
//  so we should have a single implementation that both call to

public StorageError externalStorage_queryFileExists(const char *filePath,
                                                    bool *fileExists) {
    requireParamNotNull(filePath, "dirPath");
    requireParamNotNull(fileExists, "fileExists");
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, filePath);
    if (access(pathBuffer, F_OK) == 0) {
        *fileExists = true;
    } else {
        int err = errno;
        if (errno == ENOENT) {
            *fileExists = false;
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "access() returned %i: %s", err, strerror(err));
        }
    }
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_queryFileInfo(const char *filePath, FileInfo *fileInfo) {
    requireParamNotNull(filePath, "filePath");
    requireParamNotNull(fileInfo, "fileInfo");
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, filePath);

    struct stat statResult;
    if ((stat(pathBuffer, &statResult)) != 0) {
        int err = errno;
        if (errno == ENOENT) {
            throw(STORAGE_ERROR_NOT_FOUND, "file not found: %s", filePath);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "stat() returned %i: %s", err, strerror(err));
        }
    }
    fileInfo->sizeBytes = (uint32_t) statResult.st_size;

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_createFile(const char *filePath) {
    requireParamNotNull(filePath, "dirPath");
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, filePath);
    bool fileExists;
    externalStorage_queryFileExists(pathBuffer, &fileExists);
    if (!fileExists) {
        FILE *file = fopen(pathBuffer, "w");
        if (file != NULL) {
            fclose(file);
        }
    }
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_openFile(const char *filePath,
                                             FILE **fileIn,
                                             const FileMode fileMode) {
    requireParamNotNull(filePath, "filePath");
    requireParamNotNull(fileIn, "fileIn");

    bool fileExists;
    const StorageError storageError = externalStorage_queryFileExists(filePath, &fileExists);
    if (storageError != STORAGE_ERROR_NONE) {
        throw(storageError, "could not check if file exists before opening: %s", filePath);
    }
    if (!fileExists) {
        throw(STORAGE_ERROR_NOT_FOUND, "file not found: %s", filePath);
    }

    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, filePath);

    FILE *file = fopen(pathBuffer, fileModeToString(fileMode));
    if (file == NULL) {
        int err = errno;
        if (err == ENOENT) {
            throw(STORAGE_ERROR_NOT_FOUND, "file not found: %s", filePath);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "fopen() returned %i: %s", err, strerror(err));
        }
    } else {
        *fileIn = file;
    }

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_closeFile(const FILE *fileIn) {
    requireParamNotNull(fileIn, "fileIn");

    if ((fclose(fileIn) != 0)) {
        int err = errno;
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fclose() returned %i: %s", err, strerror(err));
    }
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_readFile(const FILE *file,
                                             size_t startPosition,
                                             void *bufferIn,
                                             const uint bufferLength,
                                             uint *bytesRead) {
    requireParamNotNull(file, "file");
    requireParamNotNull(bufferIn, "bufferIn");
    requireParamNotNull(bytesRead, "bytesRead");

    int err;
    if ((err = fsetpos(file, (fpos_t *) &startPosition))) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fsetpos() returned error %i: %s", err, strerror(err));
    }
    *bytesRead = fread(bufferIn, sizeof(char), bufferLength, file);
    if ((err = ferror(file))) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fread() returned error %i: %s", err, strerror(err));
    }

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_writeFile(const FILE *file,
                                              size_t startPosition,
                                              const void *buffer,
                                              const uint bufferLength,
                                              uint *bytesWritten) {
    requireParamNotNull(file, "file");
    requireParamNotNull(buffer, "buffer");
    requireParamNotNull(bytesWritten, "bytesWritten");

    int err;
    if ((err = fsetpos(file, (fpos_t *) &startPosition))) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fsetpos() returned error %i: %s", err, strerror(err));
    }
    *bytesWritten = fwrite(buffer, sizeof(char), bufferLength, file);
    if ((err = ferror(file))) {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "fwrite() returned error %i: %s", err, strerror(err));
    }

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_moveFile(const char *filePath, const char *newFilePath) {
    requireParamNotNull(filePath, "filePath");
    requireParamNotNull(newFilePath, "newFilePath");

    char oldPathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(oldPathBuffer, filePath);

    char newPathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(newPathBuffer, newFilePath);

    if ((rename(oldPathBuffer, newPathBuffer) != 0)) {
        int err = errno;
        if (err == ENOENT) {
            throw(STORAGE_ERROR_NOT_FOUND, "file(s) not found: %s -> %s", filePath, newFilePath);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "rename() returned %i: %s", err, strerror(err));
        }
    }

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_deleteFile(const char *filePath) {
    requireParamNotNull(filePath, "filePath");
    char pathBuffer[EXTERNAL_STORAGE_MAX_PATH_LENGTH];
    getPrefixedPath(pathBuffer, filePath);
    if (remove(pathBuffer) != 0) {
        int err = errno;
        if (err == ENOENT) {
            throw(STORAGE_ERROR_NOT_FOUND, "file not found: %s", filePath);
        } else {
            throw(STORAGE_ERROR_GENERIC_FAILURE, "remove() returned %i: %s", err, strerror(err));
        }
    }
    return STORAGE_ERROR_NONE;
}