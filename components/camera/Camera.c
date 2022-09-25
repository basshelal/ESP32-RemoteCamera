#include "Utils.h"
#include "Camera.h"
#include "Logger.h"
#include <driver/spi_master.h>
#include <string.h>

#define WRITE 0x80
#define READ  0x00

// TODO: 25-Sep-2022 @basshelal: Links:
//  https://github.com/ArduCAM/Arduino/blob/master/ArduCAM/examples/RaspberryPi/arducam_ov5642_capture.cpp
//  https://github.com/ArduCAM/Arduino/blob/master/ArduCAM/ArduCAM.cpp
//  https://github.com/ArduCAM/Arduino/blob/master/ArduCAM/ArduCAM.h
//  https://www.arducam.com/downloads/shields/ArduCAM_Camera_Shield_Software_Application_Note.pdf

// TODO: 25-Sep-2022 @basshelal: We need I2C to communicate to the sensor directly

private spi_device_handle_t handle;

private Error camera_testCameraRegister(const uint8_t inputValue, uint8_t *returnValue) {
    spi_transaction_t tx = {
            .cmd = 0x00 | WRITE,
            .length = 8,
            .rxlength = 0,
            .rx_buffer = NULL,
            .tx_data = {inputValue},
            .flags = SPI_TRANS_USE_TXDATA
    };
    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);

    spi_transaction_t tx2 = {
            .cmd = 0x00,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };
    err = spi_device_polling_transmit(handle, &tx2);
    ESP_ERROR_CHECK(err);
    *returnValue = tx2.rx_data[0];

    return ERROR_NONE;
}

private Error camera_setFramesToCapture(uint8_t framesCount) {
    framesCount--;
    if (framesCount > 7) framesCount = 7;
    spi_transaction_t tx = {
            .cmd = 0x01 | WRITE,
            .length = 8,
            .rxlength = 0,
            .rx_buffer = NULL,
            .tx_data = {framesCount},
            .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_getFramesToCapture(uint8_t *framesCount) {
    spi_transaction_t tx = {
            .cmd = 0x01,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    *framesCount = ++(tx.rx_data[0]);
    return ERROR_NONE;
}

private Error camera_clearFIFODone() {
    spi_transaction_t tx = {
            .cmd = 0x04 | WRITE,
            .length = 8,
            .rxlength = 0,
            .rx_buffer = NULL,
            .tx_data = {0x01},
            .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_startCapture() {
    spi_transaction_t tx = {
            .cmd = 0x04 | WRITE,
            .length = 8,
            .rxlength = 0,
            .rx_buffer = NULL,
            .tx_data = {0x02},
            .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_resetFIFOWrite() {
    spi_transaction_t tx = {
            .cmd = 0x04 | WRITE,
            .length = 8,
            .rxlength = 0,
            .rx_buffer = NULL,
            .tx_data = {0x10},
            .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_resetFIFORead() {
    spi_transaction_t tx = {
            .cmd = 0x04 | WRITE,
            .length = 8,
            .rxlength = 0,
            .rx_buffer = NULL,
            .tx_data = {0x20},
            .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_isFIFODone(bool *isFIFODone) {
    spi_transaction_t tx = {
            .cmd = 0x41,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };
    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    uint8_t result = tx.rx_data[0];
    *isFIFODone = (result & 0x8) == 0x08;
    return ERROR_NONE;
}

private Error camera_getFIFOLength(uint32_t *const fifoLength) {
    esp_err_t err;
    spi_transaction_t tx1, tx2, tx3;
    uint8_t result1, result2, result3;

    tx1 = (spi_transaction_t) {
            .cmd = 0x42,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };
    err = spi_device_polling_transmit(handle, &tx1);
    ESP_ERROR_CHECK(err);
    result1 = tx1.rx_data[0];

    tx2 = (spi_transaction_t) {
            .cmd = 0x43,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };
    err = spi_device_polling_transmit(handle, &tx2);
    ESP_ERROR_CHECK(err);
    result2 = tx2.rx_data[0];

    tx3 = (spi_transaction_t) {
            .cmd = 0x44,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };
    err = spi_device_polling_transmit(handle, &tx3);
    ESP_ERROR_CHECK(err);
    result3 = tx3.rx_data[0];

    INFO("Camera FIFO length: %i,%i,%i", result1, result2, result3);
    return ERROR_NONE;
}

typedef enum {
    UNKNOWN = 0x0, MCU = 0x01, CAMERA = 0x02,
} DataBusOwner;

private Error camera_setDataBusOwner(const DataBusOwner owner) {
    spi_transaction_t tx = {
            .cmd = 0x02 | WRITE,
            .length = 8,
            .rxlength = 0,
            .rx_buffer = NULL,
            .tx_data = {owner},
            .flags = SPI_TRANS_USE_TXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_getDataBusOwner(DataBusOwner *owner) {
    spi_transaction_t tx = {
            .cmd = 0x02,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };
    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);

    const uint8_t result = tx.rx_data[0];
    if ((result & CAMERA) == CAMERA) {
        *owner = CAMERA;
    } else if ((result & MCU) == MCU) {
        *owner = MCU;
    } else {
        *owner = UNKNOWN;
    }

    return ERROR_NONE;
}

private Error camera_FIFORead() {
    int size = 16;
    uint8_t *buffer = alloc(size);
    spi_transaction_t tx = {
            .cmd = 0x3D,
            .rx_buffer = buffer,
            .rxlength = size * 8,
    };
    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);

    char result[1024] = "";
    char tempBuffer[32];
    for (int i = 0; i < size; i++) {
        sprintf(tempBuffer, "%u, ", buffer[i]);
        strcat(result, tempBuffer);
    }
    INFO("%s", result);
    free(buffer);
    return ERROR_NONE;
}

private Error camera_waitForFIFODone() {
    bool isDone = false;
    while (!isDone) {
        camera_isFIFODone(&isDone);
    }
    return ERROR_NONE;
}

private Error camera_getVersion(uint8_t *major, uint8_t *minor) {
    spi_transaction_t tx = {
            .cmd = 0x40,
            .rxlength = 8,
            .flags = SPI_TRANS_USE_RXDATA
    };

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    ESP_ERROR_CHECK(err);

    *major = tx.rx_data[0] >> 4;
    *minor = tx.rx_data[0] & 0x0F;

    INFO("Camera version: %i.%i", *major, *minor);

    return ERROR_NONE;
}

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
            .address_bits = 0,
            .clock_speed_hz = SPI_MASTER_FREQ_8M,
            .mode = 0,
            .spics_io_num = 5,
            .queue_size = 1,
            .flags = SPI_DEVICE_HALFDUPLEX
    };

    err = spi_bus_add_device(VSPI_HOST, &deviceConfig, &handle);
    ESP_ERROR_CHECK(err);

    camera_setFramesToCapture(1);
    uint8_t frames;
    camera_getFramesToCapture(&frames);
    INFO("Frames to capture: %u", frames);

    uint8_t returnValue;
    camera_testCameraRegister(69, &returnValue);
    INFO("returnValue: %u", returnValue);

    return ESP_OK;
}

public Error camera_destroy() {
    return ERROR_NONE;
}

public void camera_read() {
    camera_clearFIFODone();
    camera_resetFIFORead();
    camera_resetFIFOWrite();
    camera_startCapture();
    camera_waitForFIFODone();
    camera_getFIFOLength(NULL);
    camera_FIFORead();
}