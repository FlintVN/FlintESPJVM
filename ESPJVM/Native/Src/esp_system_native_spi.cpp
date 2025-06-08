
#include <string.h>
#include "flint.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "flint_system_api.h"
#include "esp_system_native_spi.h"
#include "flint_throw_support.h"

class EspSpiObject : public FlintJavaObject {
public:
    int32_t spiId() {
        return getFields().getFieldData32ByIndex(0)->value;
    }

    int32_t &mode() {
        return getFields().getFieldData32ByIndex(1)->value;
    }

    int32_t &speed() {
        return getFields().getFieldData32ByIndex(2)->value;
    }

    int32_t &maxTransferSize() {
        return getFields().getFieldData32ByIndex(3)->value;
    }

    int32_t &mosiPin() {
        return getFields().getFieldData32ByIndex(4)->value;
    }

    int32_t &misoPin() {
        return getFields().getFieldData32ByIndex(5)->value;
    }

    int32_t &clkPin() {
        return getFields().getFieldData32ByIndex(6)->value;
    }

    int32_t &csPin() {
        return getFields().getFieldData32ByIndex(7)->value;
    }
};

typedef struct {
    spi_device_handle_t handle;
    EspSpiObject *spiObj;
} EspSpiHandle;

static EspSpiHandle espSpiHandle[2] = {
    {0, 0}, {0, 0}
};

static bool NativeSPI_IsOpen(int32_t spiId) {
    return espSpiHandle[spiId - 1].handle ? true : false;
}

static esp_err_t NativeSPI_Transfer(int32_t spiId, FlintInt8Array *txBuff, int32_t txOff, FlintInt8Array *rxBuff, int32_t rxOff, int32_t length) {
    spi_transaction_t t = {};
    int8_t *txData = txBuff ? &txBuff->getData()[txOff] : NULL;
    int8_t *rxData = rxBuff ? &rxBuff->getData()[rxOff] : NULL;
    t.length = length * 8;
    t.tx_buffer = txData;
    t.rx_buffer = rxData;
    return spi_device_transmit(espSpiHandle[spiId - 1].handle, &t);
}

static void NativeSPI_Close(int32_t spiId) {
    if(NativeSPI_IsOpen(spiId)) {
        int32_t index = spiId - 1;
        spi_bus_remove_device(espSpiHandle[index].handle);
        spi_bus_free((spi_host_device_t)spiId);
        espSpiHandle[index].handle = 0;
        espSpiHandle[index].spiObj = 0;
    }
}

void NativeSPI_Reset(Flint &flint) {
    ((void)flint);
    for(uint8_t i = 0; i < LENGTH(espSpiHandle); i++)
        NativeSPI_Close(i + 1);
}

static FlintError checkSpiId(FlintExecution &execution, int32_t spiId) {
    const char *msg;
    if(spiId == 0)
        msg = "SPI1 was used for flash memory";
    else if(spiId > 2)
        msg = "spiId value is invalid";
    else
        return ERR_OK;
    return throwIOException(execution, msg);
}

static FlintError checkSpiTransferCondition(FlintExecution &execution, EspSpiObject *spiObj) {
    int32_t spiId = spiObj->spiId();
    RETURN_IF_ERR(checkSpiId(execution, spiId));
    if(!NativeSPI_IsOpen(spiId))
        return throwIOException(execution, "SPI has not been opened yet");
    else if(espSpiHandle[spiId - 1].spiObj != spiObj)
        return throwIOException(execution, "Access is denied");
    return ERR_OK;
}

static FlintError checkSpiInputParam(FlintExecution &execution, FlintInt8Array *buff, int32_t offset, int32_t count) {
    if(buff) {
        if(offset < 0)
            return throwArrayIndexOutOfBoundsException(execution, offset, buff->getLength());
        else if((offset + count) > buff->getLength())
            return throwArrayIndexOutOfBoundsException(execution, buff->getLength(), buff->getLength());
    }
    return ERR_OK;
}

static FlintError nativeInitInstance(FlintExecution &execution) {
    static const uint8_t spiMosiPin[] = {23, 13};
    static const uint8_t spiMisoPin[] = {19, 12};
    static const uint8_t spiClkPin[] = {18, 14};

    EspSpiObject *spiObj = (EspSpiObject *)execution.stackPopObject();
    int32_t spiId = spiObj->spiId();
    RETURN_IF_ERR(checkSpiId(execution, spiId));

    spiObj->mode() = 0;
    spiObj->speed() = 5000000;
    spiObj->maxTransferSize() = 4096;
    spiObj->mosiPin() = spiMosiPin[spiId - 1];
    spiObj->misoPin() = spiMisoPin[spiId - 1];
    spiObj->clkPin() = spiClkPin[spiId - 1];
    spiObj->csPin() = -1;

    return ERR_OK;
}

static FlintError nativeOpen(FlintExecution &execution) {
    EspSpiObject *spiObj = (EspSpiObject *)execution.stackPopObject();
    int32_t spiId = spiObj->spiId();
    RETURN_IF_ERR(checkSpiId(execution, spiId));

    if(NativeSPI_IsOpen(spiId)) {
        if(espSpiHandle[spiId - 1].spiObj != spiObj)
            return throwIOException(execution, "Access is denied");
        return throwIOException(execution, "SPI is already open");
    }

    int32_t mode = spiObj->mode();

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = spiObj->mosiPin();
    buscfg.miso_io_num = spiObj->misoPin();
    buscfg.sclk_io_num = spiObj->clkPin();
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = spiObj->maxTransferSize();

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = spiObj->speed();
    devcfg.mode = (mode & 0x03);
    devcfg.spics_io_num = spiObj->csPin();
    devcfg.queue_size = 1;
    devcfg.flags |= (mode & 0x04) ? SPI_DEVICE_BIT_LSBFIRST : 0;
    devcfg.flags |= (mode & 0x08) ? SPI_DEVICE_POSITIVE_CS : 0;

    if(spi_bus_initialize((spi_host_device_t)spiId, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK)
        return throwIOException(execution, "Error while initializing bus");
    if(spi_bus_add_device((spi_host_device_t)spiId, &devcfg, &espSpiHandle[spiId - 1].handle) != ESP_OK)
        return throwIOException(execution, "Error while adding device");

    espSpiHandle[spiId - 1].spiObj = spiObj;

    return ERR_OK;
}

static FlintError nativeIsOpen(FlintExecution &execution) {
    EspSpiObject *spiObj = (EspSpiObject *)execution.stackPopObject();
    int32_t spiId = spiObj->spiId();
    RETURN_IF_ERR(checkSpiId(execution, spiId));
    if(NativeSPI_IsOpen(spiId))
        execution.stackPushInt32((espSpiHandle[spiId - 1].spiObj == spiObj) ? 1 : 0);
    else
        execution.stackPushInt32(0);
    return ERR_OK;
}

static FlintError nativeGetSpeed(FlintExecution &execution) {
    EspSpiObject *spiObj = (EspSpiObject *)execution.stackPopObject();
    int32_t spiId = spiObj->spiId();
    RETURN_IF_ERR(checkSpiId(execution, spiId));

    if(NativeSPI_IsOpen(spiId)) {
        int32_t ret = 0;
        if(spi_device_get_actual_freq(espSpiHandle[spiId - 1].handle, (int *)&ret) != ESP_OK)
            return throwIOException(execution, "Error get actual spi speed");
        execution.stackPushInt32(ret * 1000);
    }
    else
        execution.stackPushInt32(spiObj->speed());
    return ERR_OK;
}

static FlintError nativeReadWrite(FlintExecution &execution) {
    int32_t length = execution.stackPopInt32();
    int32_t rxOff = execution.stackPopInt32();
    FlintInt8Array *rxBuff = (FlintInt8Array *)execution.stackPopObject();
    int32_t txOff = execution.stackPopInt32();
    FlintInt8Array *txBuff = (FlintInt8Array *)execution.stackPopObject();
    EspSpiObject *spiObj = (EspSpiObject *)execution.stackPopObject();

    RETURN_IF_ERR(checkSpiTransferCondition(execution, spiObj));
    if((txBuff == NULL) && (rxBuff == NULL))
        return throwIllegalArgumentException(execution);
    RETURN_IF_ERR(checkSpiInputParam(execution, txBuff, txOff, length));
    RETURN_IF_ERR(checkSpiInputParam(execution, rxBuff, rxOff, length));
    if(NativeSPI_Transfer(spiObj->spiId(), txBuff, txOff, rxBuff, rxOff, length) != ESP_OK)
        return throwIOException(execution, "Error while transmitting data");

    execution.stackPushInt32(rxBuff ? length : 0);
    return ERR_OK;
}

static FlintError nativeClose(FlintExecution &execution) {
    EspSpiObject *spiObj = (EspSpiObject *)execution.stackPopObject();
    int32_t spiId = spiObj->spiId();
    RETURN_IF_ERR(checkSpiId(execution, spiId));
    NativeSPI_Close(spiId);
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x0C\x00\x2F\x75""initInstance", "\x03\x00\x91\x99""()V",        nativeInitInstance),
    NATIVE_METHOD("\x04\x00\x49\xA8""open",         "\x03\x00\x91\x99""()V",        nativeOpen),
    NATIVE_METHOD("\x06\x00\x4E\xA5""isOpen",       "\x03\x00\x91\x9C""()Z",        nativeIsOpen),
    NATIVE_METHOD("\x08\x00\x63\xE0""getSpeed",     "\x03\x00\xD0\x51""()I",        nativeGetSpeed),
    NATIVE_METHOD("\x09\x00\x6E\xDB""readWrite",    "\x0A\x00\xB1\xAE""([BI[BII)I", nativeReadWrite),
    NATIVE_METHOD("\x05\x00\xD7\xA1""close",        "\x03\x00\x91\x99""()V",        nativeClose),
};

const FlintNativeClass SPI_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x15\x00\xD4\x3B""esp/machine/SpiMaster", methods);
