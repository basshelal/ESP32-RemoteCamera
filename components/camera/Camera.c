#include "Utils.h"
#include "Camera.h"
#include "Logger.h"
#include "OV5642.h"
#include <driver/spi_master.h>
#include "driver/i2c.h"
#include "TaskWatcher.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SPI_WRITE 0x80
#define SPI_READ  0x00
#define BYTE_TO_BITS 8

#define I2C_MASTER_FREQ_HZ 400000
#define SPI_MASTER_FREQ_HZ SPI_MASTER_FREQ_8M
#define SPI_MAX_TRANSFER_SIZE SOC_SPI_MAXIMUM_BUFFER_SIZE
#define I2C_DELAY_MILLIS 100

#define CAMERA_TASK_NAME "cameraTask"
#define CAMERA_TASK_STACK_SIZE 4000
#define CAMERA_TASK_STACK_MIN (CAMERA_TASK_STACK_SIZE * 0.10)
#define CAMERA_TASK_PRIORITY ((configMAX_PRIORITIES - 1)/2)
#define CAMERA_LIVE_IMAGE_BUFFER_SIZE 4096

/*
 * Arducam & Sensor are LSB so bits are in the order 76543210, so 1 in bit 1 is 00000010 or 0x02
 */

private struct {
    spi_device_handle_t spiDeviceHandle;
    SemaphoreHandle_t semaphoreHandle;
    struct {
        TaskHandle_t handle;
        bool isRunning;
        bool isPaused;
        uint32_t delayMillis;
        uint8_t *liveImageBuffer;
        size_t liveImageBufferLength;
        CameraLiveCaptureCallback *liveCaptureCallback;
    } task;
} this;

#define obtainMutex() xSemaphoreTake(this.semaphoreHandle, portMAX_DELAY)
#define releaseMutex() xSemaphoreGive(this.semaphoreHandle)

private void spiSend(const uint16_t command,
                     const uint8_t *const sendData, const size_t sendDataLength,
                     uint8_t *const receiveData, const size_t receiveDataLength) {
    spi_transaction_t tx = {
            .cmd = command,
            .tx_buffer = sendData,
            .rxlength = (size_t) receiveDataLength * BYTE_TO_BITS,
            .rx_buffer = receiveData,
    };
    if (sendDataLength > 0) {
        tx.length = sendDataLength * BYTE_TO_BITS;
    }

    spi_device_polling_transmit(this.spiDeviceHandle, &tx);
}

#define spiSendOnly(command, sendData, sendDataLength) spiSend(command, sendData, sendDataLength, NULL, 0)
#define spiReceiveOnly(command, receiveData, receiveDataLength) spiSend(command, NULL, 0, receiveData, receiveDataLength)

private void i2cWrite(const uint16_t registerAddress,
                      const uint8_t *const sendData, const size_t sendDataLength) {
    const uint8_t firstByte = registerAddress >> 8;
    const uint8_t secondByte = registerAddress & 0x00FF;
    i2c_cmd_handle_t cmdHandle = i2c_cmd_link_create();
    i2c_master_start(cmdHandle);
    i2c_master_write_byte(cmdHandle, OV5642_I2C_DEVICE_ADDRESS_WRITE, true);
    i2c_master_write_byte(cmdHandle, firstByte, true);
    i2c_master_write_byte(cmdHandle, secondByte, true);
    if (sendDataLength > 0 && sendData != NULL) {
        i2c_master_write(cmdHandle, sendData, sendDataLength, true);
    }
    i2c_master_stop(cmdHandle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmdHandle, 1000 / portTICK_RATE_MS));
    i2c_cmd_link_delete(cmdHandle);
}

private void i2cRead(const uint16_t registerAddress,
                     uint8_t *const receiveData, const size_t receiveDataLength) {
    const uint8_t firstByte = registerAddress >> 8;
    const uint8_t secondByte = registerAddress & 0x00FF;
    i2c_cmd_handle_t cmdHandle = i2c_cmd_link_create();
    i2c_master_start(cmdHandle);
    i2c_master_write_byte(cmdHandle, OV5642_I2C_DEVICE_ADDRESS_WRITE, true);
    i2c_master_write_byte(cmdHandle, firstByte, true);
    i2c_master_write_byte(cmdHandle, secondByte, true);
    i2c_master_stop(cmdHandle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmdHandle, 1000 / portTICK_RATE_MS));
    i2c_cmd_link_delete(cmdHandle);
    cmdHandle = i2c_cmd_link_create();
    i2c_master_start(cmdHandle);
    i2c_master_write_byte(cmdHandle, OV5642_I2C_DEVICE_ADDRESS_READ, true);
    if (receiveDataLength > 0 && receiveData != NULL) {
        i2c_master_read(cmdHandle, receiveData, receiveDataLength, I2C_MASTER_NACK);
    }
    i2c_master_stop(cmdHandle);
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmdHandle, 1000 / portTICK_RATE_MS));
    i2c_cmd_link_delete(cmdHandle);
}

#define i2cWriteByte(registerAddress, byte) \
do{                                         \
const uint8_t value = byte;                 \
i2cWrite(registerAddress, &value, sizeof(value)); \
delayMillis(I2C_DELAY_MILLIS);                    \
} while(0)

private void i2cWriteRegistryEntries(const OV5642RegisterEntry *registerEntries) {
    OV5642RegisterEntry *next = registerEntries;
    OV5642RegisterEntry entry = *registerEntries;
    while (entry.address != 0xFFFF && entry.value != 0xFF) {
        i2cWriteByte(entry.address, entry.value);
        next++;
        entry = *next;
    }
}

private Error camera_setTestRegister(const uint8_t value) {
    spiSendOnly(0x00 | SPI_WRITE, &value, sizeof(value));
    return ERROR_NONE;
}

private Error camera_getTestRegister(uint8_t *const value) {
    spiReceiveOnly(0x00 | SPI_READ, value, sizeof(uint8_t));

    return ERROR_NONE;
}

private Error camera_setFramesToCapture(uint8_t framesCount) {
    framesCount--;
    if (framesCount > 7) framesCount = 7;

    spiSendOnly(0x01 | SPI_WRITE, &framesCount, sizeof(framesCount));

    return ERROR_NONE;
}

private Error camera_getFramesToCapture(uint8_t *const framesCount) {
    uint8_t receivedData = 0;
    spiReceiveOnly(0x01 | SPI_READ, &receivedData, sizeof(receivedData));
    *framesCount = ++receivedData;
    return ERROR_NONE;
}

private Error camera_setVSyncPolarity(const bool isVsyncOn) {
    const uint8_t data = ((uint8_t) (isVsyncOn)) << 1;
    spiSendOnly(0x03 | SPI_WRITE, &data, sizeof(data));

    return ERROR_NONE;
}

private Error camera_getVSyncPolarity(bool *const isVsyncOn) {
    uint8_t receivedData = 0;
    spiReceiveOnly(0x03 | SPI_READ, &receivedData, sizeof(receivedData));

    const uint8_t vsyncResult = receivedData & 0x02; // vsync is bit 1
    *isVsyncOn = (bool) vsyncResult;
    return ERROR_NONE;
}

private Error camera_clearFIFOWriteDoneFlag() {
    uint8_t data = 0x01;
    spiSendOnly(0x04 | SPI_WRITE, &data, sizeof(data));
    return ERROR_NONE;
}

private Error camera_startCapture() {
    uint8_t data = 0x02;
    spiSendOnly(0x04 | SPI_WRITE, &data, sizeof(data));

    return ERROR_NONE;
}

private Error camera_resetFIFOWrite() {
    uint8_t data = 0x10;
    spiSendOnly(0x04 | SPI_WRITE, &data, sizeof(data));
    return ERROR_NONE;
}

private Error camera_resetFIFORead() {
    uint8_t data = 0x20;
    spiSendOnly(0x04 | SPI_WRITE, &data, sizeof(data));
    return ERROR_NONE;
}

private Error camera_burstFIFORead(uint8_t *const byteBuffer, const int bufferLength) {
    spi_transaction_t tx = {
            .cmd = 0x03C | SPI_READ,
            .tx_buffer = NULL,
    };
    for (int bytesRemaining = bufferLength, bytesRead = 0; bytesRemaining > 0;) {
        const int bytesToRead = bufferLength > SPI_MAX_TRANSFER_SIZE ? SPI_MAX_TRANSFER_SIZE : bufferLength;
        tx.rxlength = (size_t) bytesToRead * BYTE_TO_BITS;
        tx.rx_buffer = byteBuffer + bytesRead;
        spi_device_polling_transmit(this.spiDeviceHandle, &tx);

        bytesRemaining -= bytesToRead;
        bytesRead += bytesToRead;
    }

    return ERROR_NONE;
}

private Error camera_singleFIFORead(uint8_t *const byteReceived) {
    uint8_t receivedData = 0;
    spiReceiveOnly(0x03D | SPI_READ, &receivedData, sizeof(receivedData));

    *byteReceived = receivedData;
    return ERROR_NONE;
}

private Error camera_getVersion(uint8_t *const major, uint8_t *const minor) {
    uint8_t receivedData = 0;
    spiReceiveOnly(0x40 | SPI_READ, &receivedData, sizeof(receivedData));

    *major = receivedData >> 4;
    *minor = receivedData & 0x0F;

    return ERROR_NONE;
}

private Error camera_getFIFOWriteDoneFlag(bool *const isFIFODone) {
    uint8_t receivedData = 0;
    spiReceiveOnly(0x41 | SPI_READ, &receivedData, sizeof(receivedData));
    *isFIFODone = (receivedData & 0x08) == 0x08;
    return ERROR_NONE;
}

private Error camera_getWriteFIFOSize(uint32_t *const fifoLength) {

    uint8_t result1 = 0;
    spiReceiveOnly(0x42 | SPI_READ, &result1, sizeof(result1));

    uint8_t result2 = 0;
    spiReceiveOnly(0x43 | SPI_READ, &result2, sizeof(result2));

    uint8_t result3 = 0;
    spiReceiveOnly(0x44 | SPI_READ, &result3, sizeof(result3));

    result3 &= 0x7F; // 0x44 returns a 7 bit number, ignore bit 7

    *fifoLength = (result3 << 16) | (result2 << 8) | (result1 << 0);

    return ERROR_NONE;
}

private Error camera_waitForFIFODone() {
    bool isDone = false;
    while (!isDone) {
        camera_getFIFOWriteDoneFlag(&isDone);
    }
    return ERROR_NONE;
}

public Error camera_setImageSize(const CameraImageSize imageSize) {
    obtainMutex();
    switch (imageSize) {
        case CAMERA_IMAGE_SIZE_320x240:
            i2cWriteRegistryEntries(OV5642_320x240);
            break;
        case CAMERA_IMAGE_SIZE_640x480:
            i2cWriteRegistryEntries(OV5642_640x480);
            break;
        case CAMERA_IMAGE_SIZE_1024x768:
            i2cWriteRegistryEntries(OV5642_1024x768);
            break;
        case CAMERA_IMAGE_SIZE_1280x960:
            i2cWriteRegistryEntries(OV5642_1280x960);
            break;
        case CAMERA_IMAGE_SIZE_1600x1200:
            i2cWriteRegistryEntries(OV5642_1600x1200);
            break;
        case CAMERA_IMAGE_SIZE_2048x1536:
            i2cWriteRegistryEntries(OV5642_2048x1536);
            break;
        case CAMERA_IMAGE_SIZE_2592x1944:
            i2cWriteRegistryEntries(OV5642_2592x1944);
            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

public Error camera_setSaturation(const int saturationLevel) {
    obtainMutex();
    switch (saturationLevel) {
        case 4:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x80);
            i2cWriteByte(0x5584, 0x80);
            i2cWriteByte(0x5580, 0x02);
            break;
        case 3:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x70);
            i2cWriteByte(0x5584, 0x70);
            i2cWriteByte(0x5580, 0x02);
            break;
        case 2:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x60);
            i2cWriteByte(0x5584, 0x60);
            i2cWriteByte(0x5580, 0x02);
            break;
        case 1:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x50);
            i2cWriteByte(0x5584, 0x50);
            i2cWriteByte(0x5580, 0x02);
            break;
        case 0:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x40);
            i2cWriteByte(0x5584, 0x40);
            i2cWriteByte(0x5580, 0x02);
            break;
        case -1:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x30);
            i2cWriteByte(0x5584, 0x30);
            i2cWriteByte(0x5580, 0x02);
            break;
        case -2:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x20);
            i2cWriteByte(0x5584, 0x20);
            i2cWriteByte(0x5580, 0x02);
            break;
        case -3:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x10);
            i2cWriteByte(0x5584, 0x10);
            i2cWriteByte(0x5580, 0x02);
            break;
        case -4:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5583, 0x00);
            i2cWriteByte(0x5584, 0x00);
            i2cWriteByte(0x5580, 0x02);
            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

public Error camera_setBrightness(const int brightnessLevel) {
    obtainMutex();
    switch (brightnessLevel) {
        case 4:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x40);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 3:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x30);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 2:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x20);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 1:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x10);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 0:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x00);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x00);
            break;
        case -1:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x10);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x08);
            break;
        case -2:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x20);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x08);
            break;
        case -3:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x30);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x08);
            break;
        case -4:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5589, 0x40);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x558a, 0x08);
            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

public Error camera_setContrast(const int contrastLevel) {
    obtainMutex();
    switch (contrastLevel) {
        case 4:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x30);
            i2cWriteByte(0x5588, 0x30);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 3:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x2c);
            i2cWriteByte(0x5588, 0x2c);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 2:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x28);
            i2cWriteByte(0x5588, 0x28);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 1:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x24);
            i2cWriteByte(0x5588, 0x24);
            i2cWriteByte(0x558a, 0x00);
            break;
        case 0:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x20);
            i2cWriteByte(0x5588, 0x20);
            i2cWriteByte(0x558a, 0x00);
            break;
        case -1:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x1c);
            i2cWriteByte(0x5588, 0x1c);
            i2cWriteByte(0x558a, 0x00);
            break;
        case -2:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x18);
            i2cWriteByte(0x5588, 0x18);
            i2cWriteByte(0x558a, 0x00);
            break;
        case -3:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x14);
            i2cWriteByte(0x5588, 0x14);
            i2cWriteByte(0x558a, 0x00);
            break;
        case -4:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x04);
            i2cWriteByte(0x5587, 0x10);
            i2cWriteByte(0x5588, 0x10);
            i2cWriteByte(0x558a, 0x00);
            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

public Error camera_setHue(const int hueLevel) {
    obtainMutex();
    switch (hueLevel) {
        case -180:
        case 180:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x80);
            i2cWriteByte(0x5582, 0x00);
            i2cWriteByte(0x558a, 0x32);
            break;
        case -150:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x6f);
            i2cWriteByte(0x5582, 0x40);
            i2cWriteByte(0x558a, 0x32);
            break;
        case -120:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x40);
            i2cWriteByte(0x5582, 0x6f);
            i2cWriteByte(0x558a, 0x32);
            break;
        case -90:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x00);
            i2cWriteByte(0x5582, 0x80);
            i2cWriteByte(0x558a, 0x02);
            break;
        case -60:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x40);
            i2cWriteByte(0x5582, 0x6f);
            i2cWriteByte(0x558a, 0x02);
            break;
        case -30:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x6f);
            i2cWriteByte(0x5582, 0x40);
            i2cWriteByte(0x558a, 0x02);
            break;
        case 0:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x80);
            i2cWriteByte(0x5582, 0x00);
            i2cWriteByte(0x558a, 0x01);
            break;
        case 30:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x6f);
            i2cWriteByte(0x5582, 0x40);
            i2cWriteByte(0x558a, 0x01);
            break;
        case 60:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x40);
            i2cWriteByte(0x5582, 0x6f);
            i2cWriteByte(0x558a, 0x01);
            break;
        case 90:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x00);
            i2cWriteByte(0x5582, 0x80);
            i2cWriteByte(0x558a, 0x31);
            break;
        case 120:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x40);
            i2cWriteByte(0x5582, 0x6f);
            i2cWriteByte(0x558a, 0x31);
            break;
        case 150:
            i2cWriteByte(0x5001, 0xff);
            i2cWriteByte(0x5580, 0x01);
            i2cWriteByte(0x5581, 0x6f);
            i2cWriteByte(0x5582, 0x40);
            i2cWriteByte(0x558a, 0x31);
            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

public Error camera_setExposure(const int exposureLevel) {
    obtainMutex();
    switch (exposureLevel) {
        case -5:
            i2cWriteByte(0x3a0f, 0x10);
            i2cWriteByte(0x3a10, 0x08);
            i2cWriteByte(0x3a1b, 0x10);
            i2cWriteByte(0x3a1e, 0x08);
            i2cWriteByte(0x3a11, 0x20);
            i2cWriteByte(0x3a1f, 0x10);
            break;
        case -4:
            i2cWriteByte(0x3a0f, 0x18);
            i2cWriteByte(0x3a10, 0x10);
            i2cWriteByte(0x3a1b, 0x18);
            i2cWriteByte(0x3a1e, 0x10);
            i2cWriteByte(0x3a11, 0x30);
            i2cWriteByte(0x3a1f, 0x10);
            break;
        case -3:
            i2cWriteByte(0x3a0f, 0x20);
            i2cWriteByte(0x3a10, 0x18);
            i2cWriteByte(0x3a11, 0x41);
            i2cWriteByte(0x3a1b, 0x20);
            i2cWriteByte(0x3a1e, 0x18);
            i2cWriteByte(0x3a1f, 0x10);
            break;
        case -2:
            i2cWriteByte(0x3a0f, 0x28);
            i2cWriteByte(0x3a10, 0x20);
            i2cWriteByte(0x3a11, 0x51);
            i2cWriteByte(0x3a1b, 0x28);
            i2cWriteByte(0x3a1e, 0x20);
            i2cWriteByte(0x3a1f, 0x10);
            break;
        case -1:
            i2cWriteByte(0x3a0f, 0x30);
            i2cWriteByte(0x3a10, 0x28);
            i2cWriteByte(0x3a11, 0x61);
            i2cWriteByte(0x3a1b, 0x30);
            i2cWriteByte(0x3a1e, 0x28);
            i2cWriteByte(0x3a1f, 0x10);
            break;
        case 0:
            i2cWriteByte(0x3a0f, 0x38);
            i2cWriteByte(0x3a10, 0x30);
            i2cWriteByte(0x3a11, 0x61);
            i2cWriteByte(0x3a1b, 0x38);
            i2cWriteByte(0x3a1e, 0x30);
            i2cWriteByte(0x3a1f, 0x10);
            break;
        case 1:
            i2cWriteByte(0x3a0f, 0x40);
            i2cWriteByte(0x3a10, 0x38);
            i2cWriteByte(0x3a11, 0x71);
            i2cWriteByte(0x3a1b, 0x40);
            i2cWriteByte(0x3a1e, 0x38);
            i2cWriteByte(0x3a1f, 0x10);
            break;
        case 2:
            i2cWriteByte(0x3a0f, 0x48);
            i2cWriteByte(0x3a10, 0x40);
            i2cWriteByte(0x3a11, 0x80);
            i2cWriteByte(0x3a1b, 0x48);
            i2cWriteByte(0x3a1e, 0x40);
            i2cWriteByte(0x3a1f, 0x20);
            break;
        case 3:
            i2cWriteByte(0x3a0f, 0x50);
            i2cWriteByte(0x3a10, 0x48);
            i2cWriteByte(0x3a11, 0x90);
            i2cWriteByte(0x3a1b, 0x50);
            i2cWriteByte(0x3a1e, 0x48);
            i2cWriteByte(0x3a1f, 0x20);
            break;
        case 4:
            i2cWriteByte(0x3a0f, 0x58);
            i2cWriteByte(0x3a10, 0x50);
            i2cWriteByte(0x3a11, 0x91);
            i2cWriteByte(0x3a1b, 0x58);
            i2cWriteByte(0x3a1e, 0x50);
            i2cWriteByte(0x3a1f, 0x20);
            break;
        case 5:
            i2cWriteByte(0x3a0f, 0x60);
            i2cWriteByte(0x3a10, 0x58);
            i2cWriteByte(0x3a11, 0xa0);
            i2cWriteByte(0x3a1b, 0x60);
            i2cWriteByte(0x3a1e, 0x58);
            i2cWriteByte(0x3a1f, 0x20);
            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

public Error camera_setSharpness(const int sharpnessLevel) {
    obtainMutex();
    // TODO: 12-Nov-2022 @basshelal: Implement!
    switch (sharpnessLevel) {
        case 4:

            break;
        case 3:

            break;
        case 2:

            break;
        case 1:

            break;
        case 0:

            break;
        case -1:

            break;
        case -2:

            break;
        case -3:

            break;
        case -4:

            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

public Error camera_setImageQuality(const CameraImageQuality imageQuality) {
    obtainMutex();
    switch (imageQuality) {
        case CAMERA_IMAGE_QUALITY_NORMAL:
            i2cWriteByte(0x4407, 0x04); // average, default
            break;
        case CAMERA_IMAGE_QUALITY_LOW:
            i2cWriteByte(0x4407, 0x08); // high compression, low quality image
            break;
        case CAMERA_IMAGE_QUALITY_HIGH:
            i2cWriteByte(0x4407, 0x02); // low compression, high quality image
            break;
        default:
            break;
    }
    releaseMutex();
    return ERROR_NONE;
}

private Error camera_initBuses() {
    spi_bus_config_t spiBusConfig = {
            .miso_io_num = 19,
            .mosi_io_num = 23,
            .sclk_io_num = 18,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .max_transfer_sz = SPI_MAX_TRANSFER_SIZE,
            .flags = SPICOMMON_BUSFLAG_MASTER
    };
    esp_err_t err = spi_bus_initialize(VSPI_HOST, &spiBusConfig, SPI_DMA_DISABLED);
    ESP_ERROR_CHECK(err);

    spi_device_interface_config_t spiDeviceConfig = {
            .command_bits = 8,
            .address_bits = 0,
            .clock_speed_hz = SPI_MASTER_FREQ_HZ,
            .mode = 0,
            .spics_io_num = 5,
            .queue_size = 32,
            .flags = SPI_DEVICE_HALFDUPLEX
    };
    err = spi_bus_add_device(VSPI_HOST, &spiDeviceConfig, &this.spiDeviceHandle);
    ESP_ERROR_CHECK(err);

    i2c_config_t i2cConfig = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = 21,
            .scl_io_num = 17,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    err = i2c_param_config(I2C_NUM_0, &i2cConfig);
    ESP_ERROR_CHECK(err);

    err = i2c_set_data_mode(I2C_NUM_0, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
    ESP_ERROR_CHECK(err);

    err = i2c_driver_install(I2C_NUM_0, i2cConfig.mode, 0, 0, 0);
    ESP_ERROR_CHECK(err);

    return ERROR_NONE;
}

private Error camera_testArduchipSPI() {
    const uint8_t testValue = 0x69;
    camera_setTestRegister(testValue);

    uint8_t returnValue;
    camera_getTestRegister(&returnValue);
    if (returnValue != testValue) {
        throw(ERROR_ILLEGAL_STATE,
              "Test SPI register did not return expected result, expected: 0x%x, was 0x%x",
              testValue, returnValue);
    }
    return ERROR_NONE;
}

private Error camera_testOV5642SensorI2C() {
    uint8_t versionHigh;
    i2cRead(OV5642_I2C_REGISTER_CHIP_ID_HIGH, &versionHigh, sizeof(versionHigh));

    uint8_t versionLow;
    i2cRead(OV5642_I2C_REGISTER_CHIP_ID_LOW, &versionLow, sizeof(versionLow));

    if (versionHigh != OV5642_I2C_CHIP_ID_HIGH || versionLow != OV5642_I2C_CHIP_ID_LOW) {
        throw(ERROR_ILLEGAL_STATE,
              "Version I2C register did not return expected result, expected: (0x%x, 0x%x), was (0x%x, 0x%x)",
              OV5642_I2C_CHIP_ID_HIGH, OV5642_I2C_CHIP_ID_LOW,
              versionHigh, versionLow);
    }

    return ERROR_NONE;
}

private void camera_showOV5642TestColorBar() {
    // Test Color bar image
    i2cWriteByte(0x503d, 0x80);
    i2cWriteByte(0x503e, 0x00);
}

private void camera_taskFunction(void *arg) {
    typeof(this) *thisPtr = (typeof(this) *) arg;
    uint32_t stackMinBytes = 0;
    uint32_t imageSize;
    uint32_t frameDelay;
    uint32_t captureDelay;
    uint32_t readDelay;
    while (thisPtr->task.isRunning) {
        if ((taskWatcher_getTaskStackMinFreeBytes(CAMERA_TASK_NAME, &stackMinBytes) == ERROR_NONE) &&
            stackMinBytes < CAMERA_TASK_STACK_MIN) { // quit task if we run out of stack to avoid program crash
            ERROR("Camera task ran out of stack, most bytes used: %u", stackMinBytes);
            break;
        }
        if (!thisPtr->task.isPaused) {
            if (thisPtr->task.liveCaptureCallback && thisPtr->task.liveImageBuffer) {
                frameDelay = esp_log_early_timestamp();
                captureDelay = esp_log_early_timestamp();
                camera_captureImage(&imageSize);
                captureDelay = esp_log_early_timestamp() - captureDelay;
                obtainMutex();
                uint8_t *buffer = thisPtr->task.liveImageBuffer;
                const int bufferLength = (int) thisPtr->task.liveImageBufferLength;
                readDelay = esp_log_early_timestamp();
                for (int bytesRemaining = (int) imageSize; bytesRemaining > 0;) {
                    const int bytesToRead = bytesRemaining > bufferLength ? bufferLength : (int) bytesRemaining;
                    camera_burstFIFORead(buffer, bytesToRead);
                    bytesRemaining -= bytesToRead;
                    if (thisPtr->task.liveCaptureCallback) {
                        thisPtr->task.liveCaptureCallback(buffer, bytesToRead,
                                                          imageSize - bytesRemaining, bytesRemaining);
                    }
                }
                readDelay = esp_log_early_timestamp() - readDelay;
                releaseMutex();
                frameDelay = esp_log_early_timestamp() - frameDelay;
                INFO("cap: %u ms, read: %u ms, tot: %u ms, fps: %.2f",
                     captureDelay, readDelay, frameDelay, 1000.0F / (float) frameDelay);
            }
        }
        delayMillis(thisPtr->task.delayMillis);
    }
    taskWatcher_restartTask(CAMERA_TASK_NAME);
}

public Error camera_start() {
    this.semaphoreHandle = xSemaphoreCreateBinary();
    releaseMutex(); // FreeRTOS always starts it as obtained so we must release first
    i2cWriteByte(0x3008, 0x80); // Full sensor reset

    i2cWriteRegistryEntries(OV5642_QVGA_Preview);
    i2cWriteRegistryEntries(OV5642_JPEG_Capture_QSXGA);
    i2cWriteRegistryEntries(OV5642_320x240);

    i2cWriteByte(0x3818, 0xa8); // enable compression, vertical flip on
    i2cWriteByte(0x3621, 0x10); // enable mirror function
    i2cWriteByte(0x3801, 0xb0); // ? something to do with enable mirror function as well
    i2cWriteByte(0x5888, 0x00); // ? something to do with lens correction
    i2cWriteByte(0x5000, 0xFF); // General functions all enabled
    i2cWriteByte(0x5001, 0x7f); // Disable special effects like filters
    i2cWriteByte(0x5580, 0x00); // same as above
    // Advanced AWB Light mode
    i2cWriteByte(0x3406, 0x00);
    i2cWriteByte(0x5192, 0x04);
    i2cWriteByte(0x5191, 0xf8);
    i2cWriteByte(0x518d, 0x26);
    i2cWriteByte(0x518f, 0x42);
    i2cWriteByte(0x518e, 0x2b);
    i2cWriteByte(0x5190, 0x42);
    i2cWriteByte(0x518b, 0xd0);
    i2cWriteByte(0x518c, 0xbd);
    i2cWriteByte(0x5187, 0x18);
    i2cWriteByte(0x5188, 0x18);
    i2cWriteByte(0x5189, 0x56);
    i2cWriteByte(0x518a, 0x5c);
    i2cWriteByte(0x5186, 0x1c);
    i2cWriteByte(0x5181, 0x50);
    i2cWriteByte(0x5184, 0x20);
    i2cWriteByte(0x5182, 0x11);
    i2cWriteByte(0x5183, 0x00);
    camera_setImageSize(CAMERA_IMAGE_SIZE_DEFAULT);

    camera_setVSyncPolarity(true);
    camera_setFramesToCapture(1);
    camera_resetFIFOWrite();
    camera_resetFIFORead();

    INFO("Camera started successfully");

    return ERROR_NONE;
}

public Error camera_captureImage(uint32_t *imageSize) {
    obtainMutex();
    camera_resetFIFOWrite();
    camera_resetFIFORead();
    camera_clearFIFOWriteDoneFlag();

    camera_startCapture();
    camera_waitForFIFODone();

    camera_getWriteFIFOSize(imageSize);

    releaseMutex();
    return ERROR_NONE;
}

public Error camera_init() {
    throwIfError(camera_initBuses(), "");
    throwIfError(camera_start(), "");

    this.task.liveImageBufferLength = CAMERA_LIVE_IMAGE_BUFFER_SIZE;
    this.task.liveImageBuffer = alloc(this.task.liveImageBufferLength);
    this.task.delayMillis = 10;
    this.task.isRunning = true;
    this.task.isPaused = false;
    TaskInfo taskInfo = {
            .name = CAMERA_TASK_NAME,
            .taskFunction = camera_taskFunction,
            .stackBytes = CAMERA_TASK_STACK_SIZE,
            .taskParameter = &this,
            .taskPriority = CAMERA_TASK_PRIORITY,
            .taskHandle = this.task.handle
    };
    taskWatcher_addTask(&taskInfo);
    taskWatcher_startTask(CAMERA_TASK_NAME);

    return ERROR_NONE;
}

public Error camera_destroy() {
    return ERROR_NONE;
}

public Error camera_setCameraLiveCaptureCallback(CameraLiveCaptureCallback cameraLiveCaptureCallback) {
    this.task.liveCaptureCallback = cameraLiveCaptureCallback;
    return ERROR_NONE;
}

public Error camera_readImageBufferedWithCallback(char *buffer, const int bufferLength,
                                                  const uint32_t imageSize,
                                                  CameraReadCallback readCallback, void *userArg) {
    obtainMutex();
    for (int i = (int) imageSize; i >= 0;) {
        const int bytesToRead = imageSize > bufferLength ? bufferLength : (int) imageSize;
        camera_burstFIFORead((uint8_t *) buffer, bytesToRead);
        readCallback(buffer, bytesToRead, userArg);
        i -= bytesToRead;
    }

    releaseMutex();
    return ERROR_NONE;
}