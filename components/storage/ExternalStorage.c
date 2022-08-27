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

#define SD_CARD_PATH "/sd0"
#define AUTOMOUNT_TASK_NAME "automountTask"
#define AUTOMOUNT_TASK_STACK_SIZE 2400
#define AUTOMOUNT_TASK_STACK_MIN AUTOMOUNT_TASK_STACK_SIZE * 0.05
#define AUTOMOUNT_TASK_PRIORITY 2U
#define AUTOMOUNT_TASK_POLL_MILLIS 1000

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

    INFO("mount() returned: %i: %s", err, esp_err_to_name(err));

    if (err == ESP_OK) {
        atomic_store(&this.isMounted, true);
    } else {
        throw(STORAGE_ERROR_GENERIC_FAILURE, "");
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
        }
        if (newHasSDCard && !isMounted) {
            externalStorage_mountSDCard();
        } else if (!newHasSDCard && isMounted) {
            externalStorage_unmountSDCard();
        }
        vTaskDelay(pdMS_TO_TICKS(AUTOMOUNT_TASK_POLL_MILLIS));
    }
    taskWatcher_notifyTaskStopped(
            /*taskName=*/AUTOMOUNT_TASK_NAME,
            /*shouldRestart=*/true,
            /*remainingStackBytes=*/remaining);
    vTaskDelete(thisPtr->task.handle);
}

public StorageError externalStorage_init() {
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
    gpio_config(&gpioConfig);

    this.hasSDCard = externalStorage_hasSDCard();
    atomic_init(&this.isMounted, false);
    if (this.hasSDCard) {
        externalStorage_mountSDCard();
        sdmmc_card_print_info(stdout, this.card);
    }

    TaskInfo taskInfo = {
            .name = AUTOMOUNT_TASK_NAME,
            .stackBytes = AUTOMOUNT_TASK_STACK_SIZE,
            .startFunction = externalStorage_startAutoMountTask
    };
    taskWatcher_registerTask(&taskInfo);
    externalStorage_startAutoMountTask();

    this.isInitialized = true;
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_destroy() {
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_unmountSDCard() {
    const atomic_bool isMounted = atomic_load(&this.isMounted);
    if (!isMounted) return STORAGE_ERROR_NONE;
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

public StorageError externalStorage_getSDCardInfo(SDCardInfo *sdCardInfo) {
    return STORAGE_ERROR_NONE;
}