
#include <string.h>
#include "flint.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "flint_system_api.h"
#include "esp_system_native_spi.h"

static spi_device_handle_t spiHandle[2] = {0};

static int32_t NativeSPI_GetID(spi_device_handle_t handle) {
    for(uint32_t i = 0; i < LENGTH(spiHandle); i++) {
        if(handle == spiHandle[i])
            return i + 1;
    }
    return -1;
}

static void NativeSPI_Close(spi_device_handle_t handle) {
    int32_t spiId = NativeSPI_GetID(handle);
    spi_bus_remove_device(handle);
    spi_bus_free((spi_host_device_t)spiId);
    spiHandle[spiId - 1] = 0;
}

void NativeSPI_Reset(Flint &flint) {
    ((void)flint);
    for(uint8_t i = 0; i < LENGTH(spiHandle); i++) {
        if(spiHandle[i])
            NativeSPI_Close(spiHandle[i]);
    }
}

static void checkError(FlintExecution &execution, esp_err_t err, const char *msg) {
    if(err != ESP_OK) {
        FlintJavaString &strObj = execution.flint.newString(msg, strlen(msg));
        throw &execution.flint.newIOException(&strObj);
    }
}

static void checkSpiId(FlintExecution &execution, int32_t spiId) {
    const char *msg;
    if(spiId == 0)
        msg = "SPI1 was used for flash memory";
    else if(spiId > 2)
        msg = "spiId value is invalid";
    else
        return;
    FlintJavaString &strObj = execution.flint.newString(msg, strlen(msg));
    throw &execution.flint.newIOException(&strObj);
}

static int32_t checkSpiHandle(FlintExecution &execution, int32_t handle) {
    int32_t spiId = NativeSPI_GetID((spi_device_handle_t)handle);
    if(spiId >= 0)
        return spiId;
    FlintJavaString &strObj = execution.flint.newString(STR_AND_SIZE("handle is invalue"));
    throw &execution.flint.newIOException(&strObj);
}

static int32_t getDefaultMosiPin(int32_t spiId) {
    static const uint8_t spiMosi[] = {23, 13};
    return spiMosi[spiId - 1];
}

static int32_t getDefaultMisoPin(int32_t spiId) {
    static const uint8_t spiMiso[] = {19, 12};
    return spiMiso[spiId - 1];
}

static int32_t getDefaultClkPin(int32_t spiId) {
    static const uint8_t spiClk[] = {18, 14};
    return spiClk[spiId - 1];
}

static void nativeOpen(FlintExecution &execution) {
    int8_t csLevel = (int8_t)execution.stackPopInt32();
    int8_t isLsb = (int8_t)execution.stackPopInt32();
    int8_t mode = (int8_t)execution.stackPopInt32();
    FlintInt8Array *pins = (FlintInt8Array *)execution.stackPopObject();
    int32_t maxTranferSize = execution.stackPopInt32();
    int32_t speed = execution.stackPopInt32();
    int8_t spiId = (int8_t)execution.stackPopInt32();
    checkSpiId(execution, spiId);

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = pins->getData()[0];
    buscfg.miso_io_num = pins->getData()[1];
    buscfg.sclk_io_num = pins->getData()[2];
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = maxTranferSize;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = speed;
    devcfg.mode = mode;
    devcfg.spics_io_num = pins->getData()[3];
    devcfg.queue_size = 1;
    devcfg.flags |= isLsb ? SPI_DEVICE_BIT_LSBFIRST : 0;
    devcfg.flags |= csLevel ? SPI_DEVICE_POSITIVE_CS : 0;

    checkError(execution, spi_bus_initialize((spi_host_device_t)spiId, &buscfg, SPI_DMA_CH_AUTO), "Error while initializing bus");
    checkError(execution, spi_bus_add_device((spi_host_device_t)spiId, &devcfg, &spiHandle[spiId - 1]), "Error while adding device");
    execution.stackPushInt32((int32_t)spiHandle[spiId - 1]);
}

static void nativeGetActualSpeed(FlintExecution &execution) {
    int32_t handle = execution.stackPopInt32();
    int32_t ret = 0;
    checkSpiHandle(execution, handle);
    checkError(execution, spi_device_get_actual_freq((spi_device_handle_t)handle, (int *)&ret), "Error get actual spi speed");
    execution.stackPushInt32(ret * 1000);
}

static void nativeGetDefaultMosiPin(FlintExecution &execution) {
    int32_t spiId = execution.stackPopInt32();
    checkSpiId(execution, spiId);
    execution.stackPushInt32(getDefaultMosiPin(spiId));
}

static void nativeGetDefaultMisoPin(FlintExecution &execution) {
    int32_t spiId = execution.stackPopInt32();
    checkSpiId(execution, spiId);
    execution.stackPushInt32(getDefaultMisoPin(spiId));
}

static void nativeGetDefaultClkPin(FlintExecution &execution) {
    int32_t spiId = execution.stackPopInt32();
    checkSpiId(execution, spiId);
    execution.stackPushInt32(getDefaultClkPin(spiId));
}

static void nativeIsOpen(FlintExecution &execution) {
    int32_t spiId = execution.stackPopInt32();
    checkSpiId(execution, spiId);
    execution.stackPushInt32(spiHandle[spiId - 1] ? 1 : 0);
}

static void nativeWrite(FlintExecution &execution) {
    int32_t length = execution.stackPopInt32();
    int32_t rxOffset = execution.stackPopInt32();
    FlintInt8Array *rxBuff = (FlintInt8Array *)execution.stackPopObject();
    int32_t txOffset = execution.stackPopInt32();
    FlintInt8Array *txBuff = (FlintInt8Array *)execution.stackPopObject();
    int32_t handle = execution.stackPopInt32();
    checkSpiHandle(execution, handle);
    spi_transaction_t t = {};
    t.length = length * 8;
    t.tx_buffer = txBuff ? &txBuff->getData()[txOffset] : NULL;
    t.rx_buffer = rxBuff ? &rxBuff->getData()[rxOffset] : NULL;
    execution.stackPushInt32((spi_device_transmit((spi_device_handle_t)handle, &t) == ESP_OK) ? 1 : 0);
}

static void nativeClose(FlintExecution &execution) {
    int32_t handle = execution.stackPopInt32();
    checkSpiHandle(execution, handle);
    NativeSPI_Close((spi_device_handle_t)handle);
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x04\x00\x49\xA8""open",              "\x0B\x00\xE1\x0D""(III[BIZZ)I", nativeOpen),
    NATIVE_METHOD("\x0E\x00\xE0\x1B""getActualSpeed",    "\x04\x00\xF9\xCB""(I)I",        nativeGetActualSpeed),
    NATIVE_METHOD("\x11\x00\xFB\x27""getDefaultMosiPin", "\x04\x00\xF9\xCB""(I)I",        nativeGetDefaultMosiPin),
    NATIVE_METHOD("\x11\x00\xFB\xC9""getDefaultMisoPin", "\x04\x00\xF9\xCB""(I)I",        nativeGetDefaultMisoPin),
    NATIVE_METHOD("\x10\x00\x71\xEB""getDefaultClkPin",  "\x04\x00\xF9\xCB""(I)I",        nativeGetDefaultClkPin),
    NATIVE_METHOD("\x06\x00\x4E\xA5""isOpen",            "\x04\x00\xB8\x06""(I)Z",        nativeIsOpen),
    NATIVE_METHOD("\x05\x00\x03\xBB""write",             "\x0B\x00\x55\x5B""(I[BI[BII)Z", nativeWrite),
    NATIVE_METHOD("\x05\x00\xD7\xA1""close",             "\x04\x00\xB8\x03""(I)V",        nativeClose),
};

const FlintNativeClass SPI_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x15\x00\xF7\xE1""esp/machine/SPIMaster", methods);
