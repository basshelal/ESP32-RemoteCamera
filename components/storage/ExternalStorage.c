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

// TODO: 20-Aug-2022 @basshelal: Forever running thread/task to check if card has been inserted to automount,
//  will need to check if card has unmounted/removed to change state
//  this can be replaced with an interrupt that gets triggered when CardDetect pin changes
private StorageError externalStorage_mountSDCard() {
    return STORAGE_ERROR_NONE;
}

private void externalStorage_taskFunction(void *arg);

private void externalStorage_startAutoMountTask() {
    xTaskCreate(
            /*pvTaskCode=*/externalStorage_taskFunction,
            /*pcName=*/AUTOMOUNT_TASK_NAME,
            /*usStackDepth=*/AUTOMOUNT_TASK_STACK_SIZE,
            /*pvParameters=*/&this,
            /*uxPriority=*/AUTOMOUNT_TASK_PRIORITY,
            /*pxCreatedTask=*/this.task.handle
    );
}

private void externalStorage_taskFunction(void *arg) {
    typeof(this) *thisPtr = (typeof(this) *) arg;
    while (thisPtr->task.isRunning) {
        // TODO: 22-Aug-2022 @basshelal: Start task, this changes the state to the following:
        //  if (mounted && !hasSDCard) unmount()
        //  if (!mounted && hasSDCard) mount()
        //  if ((mounted && hasSDCard) || (!mounted && !hasSDCard)) OK
        INFO("AutoMountTask run!");
        vTaskDelay(pdMS_TO_TICKS(thisPtr->task.pollMillis));
    }
    vTaskDelete(thisPtr->task.handle);
}

public StorageError externalStorage_init() {
    esp_err_t err;
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
            .format_if_mount_failed = false,
            .max_files = 5,
    };

    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";

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

    err = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    sdmmc_card_print_info(stdout, card);

    DIR *dir = opendir(mount_point);

    if (dir != NULL) {
        struct dirent *dirent;
        while ((dirent = readdir(dir))) {
            printf("entry: %s\n", dirent->d_name);
        }
    }

    closedir(dir);

    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_destroy() {
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_unmountSDCard() {
    return STORAGE_ERROR_NONE;
}

public bool externalStorage_hasSDCard() {
    return STORAGE_ERROR_NONE;
}

public StorageError externalStorage_getSDCardInfo(SDCardInfo *sdCardInfo) {
    return STORAGE_ERROR_NONE;
}