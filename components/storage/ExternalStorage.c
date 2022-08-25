#include "ExternalStorage.h"
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <diskio_sdmmc.h>
#include <diskio_impl.h>
#include <dirent.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Logger.h"

#define SD_CARD_PATH "/sd0"
#define AUTOMOUNT_TASK_NAME "automountTask"
#define AUTOMOUNT_TASK_STACK_SIZE 2200
#define AUTOMOUNT_TASK_PRIORITY 2U

private struct {
    bool isInitialized;
    sdmmc_host_t *sdmmcHost;
    // set to true from thread,
    // set to false from thread or externalStorage_unmountSDCard()
    // TODO: 20-Aug-2022 @basshelal: Perhaps this should be atomicBool
    bool isMounted;
    bool hasSDCard;
    struct {
        TaskHandle_t handle;
        bool isRunning;
        uint pollMillis;
    } task;
} this;

private StorageError externalStorage_mountSDCard() {
    return STORAGE_ERROR_NONE;
}

private void externalStorage_autoMountTaskFunction(void *arg);

private void externalStorage_startAutoMountTask() {
    this.task.isRunning = true;
    this.task.pollMillis = 1000;
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
    typeof(this) *thisPtr = (typeof(this) *) arg;
    while (thisPtr->task.isRunning) {
        const uint remaining = uxTaskGetStackHighWaterMark(thisPtr->task.handle);
        if (remaining < 128) { // quit task if we run out of stack to avoid program crash
            ERROR("Automount task ran out of stack, bytes remaining: %u", remaining);
            // TODO: 11-Aug-2022 @basshelal: Notify the main task that we need to restart, we can't do it from here
            break;
        }
        const bool oldHasSDCard = thisPtr->hasSDCard;
        const bool newHasSDCard = externalStorage_hasSDCard();
        if (oldHasSDCard != newHasSDCard) {
            const bool isMounted = thisPtr->isMounted;
            if (newHasSDCard && !isMounted) {
                externalStorage_mountSDCard();
            } else if (!newHasSDCard && isMounted) {
                externalStorage_unmountSDCard();
            }
            thisPtr->hasSDCard = newHasSDCard;
            INFO("SD CARD: %s", boolToString(newHasSDCard));
        }
        vTaskDelay(pdMS_TO_TICKS(thisPtr->task.pollMillis));
    }
    vTaskDelete(thisPtr->task.handle);
}

public StorageError externalStorage_init() {
    esp_err_t err;
    this.hasSDCard = externalStorage_hasSDCard();
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
    };

    sdmmc_card_t *card;
    const char *mountPoint = SD_CARD_PATH;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
            .mosi_io_num = 12,
            .miso_io_num = 13,
            .sclk_io_num = 14,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = 4096,
    };

    err = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = 15;
    slot_config.host_id = host.slot;

    err = esp_vfs_fat_sdspi_mount(mountPoint, &host, &slot_config, &mount_config, &card);

    if (err == ESP_OK) {
        this.isMounted = true;
    }

    sdmmc_card_print_info(stdout, card);

    DIR *dir = opendir(mountPoint);

    if (dir != NULL) {
        struct dirent *dirent;
        while ((dirent = readdir(dir))) {
            printf("entry: %s\n", dirent->d_name);
        }
        closedir(dir);
    }

    gpio_config_t gpioConfig = {
            .pin_bit_mask = GPIO_SEL_36,
            .mode = GPIO_MODE_INPUT,
            .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&gpioConfig);
    externalStorage_startAutoMountTask();

    this.isInitialized = true;
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_destroy() {
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_unmountSDCard() {
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