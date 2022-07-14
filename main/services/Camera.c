#include "Camera.h"
#include <driver/spi_master.h>
#include <esp_log.h>

private spi_device_handle_t handle;

public esp_err_t camera_init() {

    spi_bus_config_t busConfig = {
            .miso_io_num = 19,
            .mosi_io_num = 23,
            .sclk_io_num = 18,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .flags = SPICOMMON_BUSFLAG_MASTER
    };

    esp_err_t err = spi_bus_initialize(VSPI_HOST, &busConfig, SPI_DMA_DISABLED);

    ESP_ERROR_CHECK(err);

    spi_device_interface_config_t deviceConfig = {
            .command_bits = 8,
            .clock_speed_hz = SPI_MASTER_FREQ_8M,
            .mode = 0,
            .spics_io_num = 5,
            .queue_size = 1,
            .flags = SPI_DEVICE_HALFDUPLEX
    };

    err = spi_bus_add_device(VSPI_HOST, &deviceConfig, &handle);
    ESP_ERROR_CHECK(err);

    return ESP_OK;
}

public void camera_read() {
    spi_transaction_t tx = {
            .cmd = 0x40,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);

    ESP_ERROR_CHECK(err);

    ESP_LOGI("Camera", "RxData: %i, %i, %i, %i", tx.rx_data[0], tx.rx_data[1], tx.rx_data[2], tx.rx_data[3]);
}