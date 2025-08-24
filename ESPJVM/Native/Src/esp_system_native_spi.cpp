
#include <string.h>
#include "flint.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "flint_system_api.h"
#include "esp_system_native_pin.h"
#include "esp_system_native_spi.h"

typedef class : public JObject {
public:
    int32_t spiId() {
        return getField32ByIndex(0)->value;
    }

    int32_t &mode() {
        return getField32ByIndex(1)->value;
    }

    int32_t &speed() {
        return getField32ByIndex(2)->value;
    }

    int32_t &maxTransferSize() {
        return getField32ByIndex(3)->value;
    }

    int32_t &mosiPin() {
        return getField32ByIndex(4)->value;
    }

    int32_t &misoPin() {
        return getField32ByIndex(5)->value;
    }

    int32_t &clkPin() {
        return getField32ByIndex(6)->value;
    }

    int32_t &csPin() {
        return getField32ByIndex(7)->value;
    }
} *EspSpiObject;

typedef struct {
    spi_device_handle_t handle;
    EspSpiObject spiObj;
} EspSpiHandle;

static EspSpiHandle espSpiHandle[2] = {
    {0, 0}, {0, 0}
};

static bool NativeSPI_IsOpen(int32_t spiId) {
    return espSpiHandle[spiId - 1].handle ? true : false;
}

static esp_err_t NativeSPI_Transfer(int32_t spiId, JInt8Array *txBuff, int32_t txOff, JInt8Array *rxBuff, int32_t rxOff, int32_t length) {
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
        espSpiHandle[index].handle = NULL;
        espSpiHandle[index].spiObj = NULL;
    }
}

void NativeSPI_Reset() {
    for(uint8_t i = 0; i < LENGTH(espSpiHandle); i++)
        NativeSPI_Close(i + 1);
}

static bool checkSpiId(FNIEnv *env, int32_t spiId) {
    const char *msg;
    if(spiId == 0)
        msg = "SPI1 was used for flash memory";
    else if(spiId > 2)
        msg = "spiId value is invalid";
    else
        return true;
    env->throwNew(env->findClass("java/io/IOException"), msg);
    return false;
}

static bool checkSpiPin(FNIEnv *env, EspSpiObject spiObj) {
    const char *msg;
    if(spiObj->mosiPin() < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "MOSI pin value cannot be -1 when in SPI Master mode");
        return false;
    }
    int32_t miso = spiObj->misoPin();
    if(miso >= 0) {
        if((msg = NativePin_CheckPin(miso)) != NULL) {
            env->throwNew(env->findClass("java/io/IOException"), msg);
            return false;
        }
    }
    int32_t clk = spiObj->clkPin();
    if(clk < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "CLK pin value cannot be -1 when in SPI Master mode");
        return false;
    }
    if((msg = NativePin_CheckPin(clk)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    int32_t cs = spiObj->csPin();
    if(cs >= 0) {
        if((msg = NativePin_CheckPin(cs)) != NULL) {
            env->throwNew(env->findClass("java/io/IOException"), msg);
            return false;
        }
    }
    return true;
}

static bool checkSpiTransferCondition(FNIEnv *env, EspSpiObject spiObj) {
    int32_t spiId = spiObj->spiId();
    if(!checkSpiId(env, spiId)) return false;
    if(!NativeSPI_IsOpen(spiId)) {
        env->throwNew(env->findClass("java/io/IOException"), "SPI has not been opened yet");
        return false;
    }
    else if(espSpiHandle[spiId - 1].spiObj != spiObj) {
        env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
        return false;
    }
    return true;
}

static bool checkSpiInputParam(FNIEnv *env, jbyteArray buff, int32_t offset, int32_t count) {
    if(buff != NULL) {
        if(offset < 0) {
            jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
            uint16_t len;
            const char *name = buff->getCompTypeName(&len);
            env->throwNew(excpCls, "index %d out of bounds for %.*s[%d]", offset, len, name, buff->getLength());
            return false;
        }
        else if((offset + count) > buff->getLength()) {
            jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
            uint16_t len;
            const char *name = buff->getCompTypeName(&len);
            env->throwNew(excpCls, "last index %d out of bounds for %.*s[%d]", offset + count, len, name, buff->getLength());
            return false;
        }
    }
    return true;
}

jvoid nativeSpiInitInstance(FNIEnv *env, jobject obj) {
    static const uint8_t spiMosiPin[] = {23, 13};
    static const uint8_t spiMisoPin[] = {19, 12};
    static const uint8_t spiClkPin[] = {18, 14};

    EspSpiObject spiObj = (EspSpiObject)obj;
    int32_t spiId = spiObj->spiId();
    if(!checkSpiId(env, spiId)) return;

    spiObj->mode() = 0;
    spiObj->speed() = 5000000;
    spiObj->maxTransferSize() = 4096;
    spiObj->mosiPin() = spiMosiPin[spiId - 1];
    spiObj->misoPin() = spiMisoPin[spiId - 1];
    spiObj->clkPin() = spiClkPin[spiId - 1];
    spiObj->csPin() = -1;
}

jvoid nativeSpiOpen(FNIEnv *env, jobject obj) {
    EspSpiObject spiObj = (EspSpiObject)obj;
    int32_t spiId = spiObj->spiId();
    if(!checkSpiId(env, spiId)) return;

    if(NativeSPI_IsOpen(spiId)) {
        if(espSpiHandle[spiId - 1].spiObj != spiObj)
            env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
        else
            env->throwNew(env->findClass("java/io/IOException"), "SPI is already open");
        return;
    }

    if(!checkSpiPin(env, spiObj)) return;

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

    if(spi_bus_initialize((spi_host_device_t)spiId, &buscfg, SPI_DMA_CH_AUTO) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while initializing bus");
        return;
    }
    if(spi_bus_add_device((spi_host_device_t)spiId, &devcfg, &espSpiHandle[spiId - 1].handle) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while adding device");
        return;
    }

    espSpiHandle[spiId - 1].spiObj = spiObj;
}

jbool nativeSpiIsOpen(FNIEnv *env, jobject obj) {
    EspSpiObject spiObj = (EspSpiObject)obj;
    int32_t spiId = spiObj->spiId();
    if(!checkSpiId(env, spiId)) return false;
    if(NativeSPI_IsOpen(spiId))
        return (espSpiHandle[spiId - 1].spiObj == spiObj) ? true : false;
    return false;
}

jint nativeSpiGetSpeed(FNIEnv *env, jobject obj) {
    EspSpiObject spiObj = (EspSpiObject)obj;
    int32_t spiId = spiObj->spiId();
    if(!checkSpiId(env, spiId)) return 0;

    if(NativeSPI_IsOpen(spiId)) {
        int32_t ret = 0;
        if(spi_device_get_actual_freq(espSpiHandle[spiId - 1].handle, (int *)&ret) != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Error get actual spi speed");
            return 0;
        }
        return ret * 1000;
    }
    return spiObj->speed();
}

jint nativeSpiReadWrite(FNIEnv *env, jobject obj, jbyteArray txBuff, jint txOff, jboolArray rxBuff, jint rxOff, jint length) {
    EspSpiObject spiObj = (EspSpiObject)obj;
    if(!checkSpiTransferCondition(env, spiObj)) return 0;
    if((txBuff == NULL) && (rxBuff == NULL)) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return 0;
    }
    if(!checkSpiInputParam(env, txBuff, txOff, length)) return 0;
    if(!checkSpiInputParam(env, rxBuff, rxOff, length)) return 0;
    if(NativeSPI_Transfer(spiObj->spiId(), txBuff, txOff, rxBuff, rxOff, length) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"),"Error while transmitting data");
        return 0;
    }

    return rxBuff ? length : 0;
}

jvoid nativeSpiClose(FNIEnv *env, jobject obj) {
    EspSpiObject spiObj = (EspSpiObject)obj;
    int32_t spiId = spiObj->spiId();
    if(!checkSpiId(env, spiId)) return;
    NativeSPI_Close(spiId);
}
