#include "Utils.h"
#include "Camera.h"
#include "Logger.h"
#include "OV5642.h"
#include <driver/spi_master.h>
#include "driver/i2c.h"
#include <string.h>

#define WRITE 0x80
#define READ  0x00
#define BYTE_TO_BITS 8

#define I2C_MASTER_FREQ_HZ 400000

/*
 * Arducam is LSB so bits are in the order 76543210, so 1 in bit 1 is 00000010 or 0x02
 */

private spi_device_handle_t handle;

private esp_err_t spiSend(const uint16_t command,
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

    esp_err_t err = spi_device_polling_transmit(handle, &tx);
    return err;
}

#define spiSendOnly(command, sendData, sendDataLength) spiSend(command, sendData, sendDataLength, NULL, 0)
#define spiReceiveOnly(command, receiveData, receiveDataLength) spiSend(command, NULL, 0, receiveData, receiveDataLength)

private void i2cWrite(const uint16_t registerAddress,
                      const uint8_t *const sendData, const size_t sendDataLength) {
    const uint8_t firstByte = registerAddress >> 8;
    const uint8_t secondByte = registerAddress & 0x00FF;
    i2c_cmd_handle_t cmdHandle = i2c_cmd_link_create();
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_start(cmdHandle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmdHandle, OV5642_I2C_DEVICE_ADDRESS_WRITE, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmdHandle, firstByte, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmdHandle, secondByte, true));
    if (sendDataLength > 0) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write(cmdHandle, sendData, sendDataLength, true));
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_stop(cmdHandle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmdHandle, pdMS_TO_TICKS(1000)));
    i2c_cmd_link_delete(cmdHandle);
}

private void i2cRead(const uint16_t registerAddress,
                     uint8_t *const receiveData, const size_t receiveDataLength) {
    const uint8_t firstByte = registerAddress >> 8;
    const uint8_t secondByte = registerAddress & 0x00FF;
    i2c_cmd_handle_t cmdHandle = i2c_cmd_link_create();
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_start(cmdHandle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmdHandle, OV5642_I2C_DEVICE_ADDRESS_READ, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmdHandle, firstByte, true));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_write_byte(cmdHandle, secondByte, true));
    if (receiveDataLength > 0) {
        ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_read(cmdHandle, receiveData, receiveDataLength, I2C_MASTER_NACK));
    }
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_stop(cmdHandle));
    ESP_ERROR_CHECK_WITHOUT_ABORT(i2c_master_cmd_begin(I2C_NUM_0, cmdHandle, pdMS_TO_TICKS(1000)));
    i2c_cmd_link_delete(cmdHandle);
}

#define LITTLETOBIG(x)          ((x<<8)|(x>>8))
#define SCCB_FREQ               400000                /*!< I2C master frequency*/
#define WRITE_BIT               I2C_MASTER_WRITE      /*!< I2C master write */
#define READ_BIT                I2C_MASTER_READ       /*!< I2C master read */
#define ACK_CHECK_EN            0x1                   /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS           0x0                   /*!< I2C master will not check ack from slave */
#define ACK_VAL                 0x0                   /*!< I2C ack value */
#define NACK_VAL                0x1                   /*!< I2C nack value */

private uint8_t new_SCCB_Read(uint8_t slv_addr, uint8_t reg) {
    uint8_t data = 0;
    esp_err_t ret = ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slv_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) return -1;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slv_addr << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ERROR("SCCB_Read Failed addr:0x%02x, reg:0x%02x, data:0x%02x, ret:%d", slv_addr, reg, data, ret);
    }
    return data;
}

private uint8_t new_SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data) {
    esp_err_t ret = ESP_FAIL;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slv_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ERROR("SCCB_Write Failed addr:0x%02x, reg:0x%02x, data:0x%02x, ret:%d", slv_addr, reg, data, ret);
    }
    return ret == ESP_OK ? 0 : -1;
}

private uint8_t new_SCCB_Read16(uint8_t slv_addr, uint16_t reg) {
    uint8_t data = 0;
    esp_err_t ret = ESP_FAIL;
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *reg_u8 = (uint8_t *) &reg_htons;
    INFO("first: 0x%x, second: 0x%x", reg_u8[0], reg_u8[1]);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slv_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[0], ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[1], ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) return -1;
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slv_addr << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, NACK_VAL);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ERROR("W [%04x]=%02x fail\n", reg, data);
    }
    return data;
}

private uint8_t new_SCCB_Write16(uint8_t slv_addr, uint16_t reg, uint8_t data) {
    static uint16_t i = 0;
    esp_err_t ret = ESP_FAIL;
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *reg_u8 = (uint8_t *) &reg_htons;
    INFO("first: 0x%x, second: 0x%x", reg_u8[0], reg_u8[1]);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (slv_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[0], ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_u8[1], ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK) {
        ERROR("W [%04x]=%02x %d fail\n", reg, data, i++);
    }
    return ret == ESP_OK ? 0 : -1;
}


private Error camera_setTestRegister(const uint8_t value) {
    esp_err_t err = spiSendOnly(0x00 | WRITE, &value, sizeof(value));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_getTestRegister(uint8_t *const value) {
    esp_err_t err = spiReceiveOnly(0x00 | READ, value, sizeof(uint8_t));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_setFramesToCapture(uint8_t framesCount) {
    framesCount--;
    if (framesCount > 7) framesCount = 7;

    esp_err_t err = spiSendOnly(0x01 | WRITE, &framesCount, sizeof(framesCount));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_getFramesToCapture(uint8_t *const framesCount) {
    uint8_t receivedData = 0;
    esp_err_t err = spiReceiveOnly(0x01 | READ, &receivedData, sizeof(receivedData));
    ESP_ERROR_CHECK(err);
    *framesCount = ++receivedData;
    return ERROR_NONE;
}

typedef enum {
    UNKNOWN = 0x0, MCU = 0x01, CAMERA = 0x02,
} DataBusOwner;

private Error camera_setDataBusOwner(const DataBusOwner owner) {
    const uint8_t data = (uint8_t) owner;
    esp_err_t err = spiSendOnly(0x02 | WRITE, &data, sizeof(data));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_getDataBusOwner(DataBusOwner *const owner) {
    uint8_t receivedData = 0;
    esp_err_t err = spiReceiveOnly(0x02 | READ, &receivedData, sizeof(receivedData));
    ESP_ERROR_CHECK(err);

    if ((receivedData & CAMERA) == CAMERA) {
        *owner = CAMERA;
    } else if ((receivedData & MCU) == MCU) {
        *owner = MCU;
    } else {
        *owner = UNKNOWN;
    }

    return ERROR_NONE;
}

private Error camera_setHSyncPolarity();
private Error camera_getHSyncPolarity();

private Error camera_setVSyncPolarity();
private Error camera_getVSyncPolarity();

private Error camera_setLCDBacklight();
private Error camera_getLCDBacklight();

private Error camera_setSensorPClock();
private Error camera_getSensorPClock();

private Error camera_clearFIFOWriteDoneFlag() {
    uint8_t data = 0x01;
    esp_err_t err = spiSendOnly(0x04 | WRITE, &data, sizeof(data));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_startCapture() {
    uint8_t data = 0x02;
    esp_err_t err = spiSendOnly(0x04 | WRITE, &data, sizeof(data));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_resetFIFOWrite() {
    uint8_t data = 0x010;
    esp_err_t err = spiSendOnly(0x04 | WRITE, &data, sizeof(data));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_resetFIFORead() {
    uint8_t data = 0x020;
    esp_err_t err = spiSendOnly(0x04 | WRITE, &data, sizeof(data));
    ESP_ERROR_CHECK(err);
    return ERROR_NONE;
}

private Error camera_resetSensorIODirection();

private Error camera_setSensorPowerDownIODirection();
private Error camera_getSensorPowerDownIODirection();

private Error camera_setSensorPowerEnableIODirection();
private Error camera_getSensorPowerEnableIODirection();

private Error camera_burstFIFORead();
private Error camera_singleFIFORead();

// TODO: 30-Sep-2022 @basshelal: Missing here is LCD Control Register, I need to figure out what it does/means

private Error camera_getVersion(uint8_t *const major, uint8_t *const minor) {
    uint8_t receivedData = 0;
    esp_err_t err = spiReceiveOnly(0x40 | READ, &receivedData, sizeof(receivedData));
    ESP_ERROR_CHECK(err);

    *major = receivedData >> 4;
    *minor = receivedData & 0x0F;

    return ERROR_NONE;
}

private Error camera_getVSyncPinStatus();

private Error camera_getFIFOWriteDoneFlag(bool *const isFIFODone) {
    uint8_t receivedData = 0;
    esp_err_t err = spiReceiveOnly(0x41 | READ, &receivedData, sizeof(receivedData));
    ESP_ERROR_CHECK(err);
    *isFIFODone = (receivedData & 0x08) == 0x08;
    return ERROR_NONE;
}

private Error camera_getWriteFIFOSize(uint32_t *const fifoLength) {
    esp_err_t err;

    uint8_t result1 = 0;
    err = spiReceiveOnly(0x42 | READ, &result1, sizeof(result1));
    ESP_ERROR_CHECK(err);

    uint8_t result2 = 0;
    err = spiReceiveOnly(0x43 | READ, &result2, sizeof(result2));
    ESP_ERROR_CHECK(err);

    uint8_t result3 = 0;
    err = spiReceiveOnly(0x44 | READ, &result3, sizeof(result3));
    ESP_ERROR_CHECK(err);

    INFO("Camera FIFO length: %i,%i,%i", result1, result2, result3);
    return ERROR_NONE;
}

private Error camera_waitForFIFODone() {
    bool isDone = false;
    while (!isDone) {
        camera_getFIFOWriteDoneFlag(&isDone);
    }
    return ERROR_NONE;
}

private Error camera_initBuses() {
    spi_bus_config_t spiBusConfig = {
            .miso_io_num = 19,
            .mosi_io_num = 23,
            .sclk_io_num = 18,
            .quadwp_io_num = -1,
            .quadhd_io_num = -1,
            .flags = SPICOMMON_BUSFLAG_MASTER
    };
    esp_err_t err = spi_bus_initialize(VSPI_HOST, &spiBusConfig, SPI_DMA_DISABLED);
    ESP_ERROR_CHECK(err);

    spi_device_interface_config_t spiDeviceConfig = {
            .command_bits = 8,
            .address_bits = 0,
            .clock_speed_hz = SPI_MASTER_FREQ_8M,
            .mode = 0,
            .spics_io_num = 5,
            .queue_size = 1,
            .flags = SPI_DEVICE_HALFDUPLEX
    };
    err = spi_bus_add_device(VSPI_HOST, &spiDeviceConfig, &handle);
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

private Error camera_initCameraConfig() {
    camera_setFramesToCapture(1);
    uint8_t frames;
    camera_getFramesToCapture(&frames);
    INFO("Frames to capture: %u", frames);

    const uint8_t testValue = 0x69;
    camera_setTestRegister(testValue);

    uint8_t returnValue;
    camera_getTestRegister(&returnValue);
    if (returnValue != testValue) {
        ERROR("Test register did not return expected result, expected: %u, was %u", testValue, returnValue);
    }

    camera_setDataBusOwner(0x0);

    uint8_t data = 0x01;
    new_SCCB_Write16(0x3C, 0x00FF, 0x01);
//    i2cWrite(0x00FF, &data, sizeof(data));

    uint8_t vid = new_SCCB_Read16(0x3C, 0x300A);
//    i2cRead(0x300A, &vid, sizeof(vid));

    uint8_t pid = new_SCCB_Read16(0x3C, 0x300B);
//    i2cRead(0x300B, &pid, sizeof(pid));

    INFO("vid: 0x%x pid: 0x%x", vid, pid);

    return ERROR_NONE;
}

public Error camera_init() {
    throwIfError(camera_initBuses(), "");
    throwIfError(camera_initCameraConfig(), "");

    return ERROR_NONE;
}

public Error camera_destroy() {
    return ERROR_NONE;
}

public void camera_read() {
//    camera_startCapture();
//    camera_waitForFIFODone();
//    camera_getWriteFIFOSize(NULL);
//    camera_FIFORead();
}