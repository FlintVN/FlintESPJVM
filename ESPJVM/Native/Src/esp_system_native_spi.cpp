
#include <string.h>
#include "flint.h"
#include "driver/spi_master.h"
#include "flint_system_api.h"
#include "esp_system_native_pin.h"
#include "esp_system_native_spi.h"

typedef class : public JObject {
public:
    jstring getSpiName() { return (jstring)getFieldByIndex(0)->getObj(); }
    int32_t getSpiId() { return getFieldByIndex(1)->getInt32(); }
    int32_t getMode() { return getFieldByIndex(2)->getInt32(); }
    int32_t getSpeed() { return getFieldByIndex(3)->getInt32(); }
    int32_t getMosi() { return getFieldByIndex(4)->getInt32(); }
    int32_t getMiso() { return getFieldByIndex(5)->getInt32(); }
    int32_t getClk() { return getFieldByIndex(6)->getInt32(); }
    int32_t getCs() { return getFieldByIndex(7)->getInt32(); }

    void setSpiId(int32_t val) { getFieldByIndex(1)->setInt32(val); }
    void setSpeed(int32_t val) { getFieldByIndex(3)->setInt32(val); }
    void setMosi(int32_t val) { getFieldByIndex(4)->setInt32(val); }
    void setMiso(int32_t val) { getFieldByIndex(5)->setInt32(val); }
    void setClk(int32_t val) { getFieldByIndex(6)->setInt32(val); }
    void setCs(int32_t val) { getFieldByIndex(7)->setInt32(val); }
} *SpiObject;

typedef struct {
    spi_device_handle_t handle;
    SpiObject spiObj;
} EspSpiHandle;

static EspSpiHandle espSpiHandle[2] = {
    {0, 0}, {0, 0}
};

static bool NativeSPI_IsOpen(int32_t spiId) {
    if(spiId == 1 || spiId == 2)
        return espSpiHandle[spiId - 1].handle ? true : false;
    return false;
}

static esp_err_t NativeSPI_Transfer(int32_t spiId, uint8_t *txBuff, int32_t txOff, uint8_t *rxBuff, int32_t rxOff, int32_t length) {
    spi_transaction_t t = {};
    t.length = length * 8;
    t.tx_buffer = txBuff;
    t.rx_buffer = rxBuff;
    return spi_device_polling_transmit(espSpiHandle[spiId - 1].handle, &t);
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

static bool checkSpiPin(FNIEnv *env, SpiObject spiObj) {
    const char *msg;
    if(spiObj->getMosi() < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "MOSI pin value cannot be -1 when in SPI Master mode");
        return false;
    }
    int32_t miso = spiObj->getMiso();
    if(miso >= 0) {
        if((msg = NativePin_CheckPin(miso)) != NULL) {
            env->throwNew(env->findClass("java/io/IOException"), msg);
            return false;
        }
    }
    int32_t clk = spiObj->getClk();
    if(clk < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "CLK pin value cannot be -1 when in SPI Master mode");
        return false;
    }
    if((msg = NativePin_CheckPin(clk)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    int32_t cs = spiObj->getCs();
    if(cs >= 0) {
        if((msg = NativePin_CheckPin(cs)) != NULL) {
            env->throwNew(env->findClass("java/io/IOException"), msg);
            return false;
        }
    }
    return true;
}

static bool checkSpiTransferCondition(FNIEnv *env, SpiObject spiObj) {
    int32_t spiId = spiObj->getSpiId();
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
            env->throwNew(excpCls, "last index %d out of bounds for %.*s[%d]", offset + count - 1, len, name, buff->getLength());
            return false;
        }
    }
    return true;
}

jobject nativeSpiOpen(FNIEnv *env, jobject obj) {
    static const uint8_t spiMosiPin[] = {23, 13};
    static const uint8_t spiMisoPin[] = {19, 12};
    static const uint8_t spiClkPin[] = {18, 14};

    SpiObject spiObj = (SpiObject)obj;
    int32_t spiId;
    jstring spiName = spiObj->getSpiName();
    if(spiName->compareTo("SPI0", strlen("SPI0")) == 0 || spiName->compareTo("SPI1", strlen("SPI1")) == 0) {
        env->throwNew(env->findClass("java/io/IOException"), "SPI0/SPI1 was used for Flash/PSRAM memory");
        return obj;
    }
    else if(spiName->compareTo("SPI2", strlen("SPI2")) == 0 || spiName->compareTo("HSPI", strlen("HSPI")) == 0)
        spiId = SPI2_HOST;
    else if(spiName->compareTo("SPI3", strlen("SPI3")) == 0 || spiName->compareTo("VSPI", strlen("VSPI")) == 0)
        spiId = SPI3_HOST;
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid SPI name");
        return obj;
    }

    spiObj->setSpiId(spiId);
    if(spiObj->getSpeed() < 0) spiObj->setSpeed(5000000);
    if(spiObj->getMosi() == -2) spiObj->setMosi(spiMosiPin[spiId - 1]);
    if(spiObj->getMiso() == -2) spiObj->setMiso(spiMisoPin[spiId - 1]);
    if(spiObj->getClk() == -2) spiObj->setClk(spiClkPin[spiId - 1]);
    if(spiObj->getCs() == -2) spiObj->setCs(-1);

    if(NativeSPI_IsOpen(spiId)) {
        const char *msg = (espSpiHandle[spiId - 1].spiObj != spiObj) ? "Access is denied" : "SPI is already open";
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return obj;
    }

    if(!checkSpiPin(env, spiObj)) return obj;

    int32_t mode = spiObj->getMode();

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = spiObj->getMosi();
    buscfg.miso_io_num = spiObj->getMiso();
    buscfg.sclk_io_num = spiObj->getClk();
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = spiObj->getSpeed();
    devcfg.mode = (mode & 0x03);
    devcfg.spics_io_num = spiObj->getCs();
    devcfg.queue_size = 1;
    devcfg.flags |= (mode & 0x04) ? SPI_DEVICE_BIT_LSBFIRST : 0;
    devcfg.flags |= (mode & 0x08) ? SPI_DEVICE_POSITIVE_CS : 0;

    if(spi_bus_initialize((spi_host_device_t)spiId, &buscfg, SPI_DMA_DISABLED) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while initializing bus");
        return obj;
    }
    if(spi_bus_add_device((spi_host_device_t)spiId, &devcfg, &espSpiHandle[spiId - 1].handle) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while adding device");
        return obj;
    }

    espSpiHandle[spiId - 1].spiObj = spiObj;
    return obj;
}

jbool nativeSpiIsOpen(FNIEnv *env, jobject obj) {
    SpiObject spiObj = (SpiObject)obj;
    int32_t spiId = spiObj->getSpiId();
    if(NativeSPI_IsOpen(spiId))
        return (espSpiHandle[spiId - 1].spiObj == spiObj) ? true : false;
    return false;
}

jint nativeSpiGetSpeed(FNIEnv *env, jobject obj) {
    SpiObject spiObj = (SpiObject)obj;
    int32_t spiId = spiObj->getSpiId();
    if(NativeSPI_IsOpen(spiId)) {
        int32_t ret = 0;
        if(spi_device_get_actual_freq(espSpiHandle[spiId - 1].handle, (int *)&ret) != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Error get actual spi speed");
            return 0;
        }
        return ret * 1000;
    }
    return spiObj->getSpeed();
}

jint nativeSpiRead(FNIEnv *env, jobject obj) {
    uint8_t buff;
    SpiObject spiObj = (SpiObject)obj;
    if(!checkSpiTransferCondition(env, spiObj)) return 0;
    if(NativeSPI_Transfer(spiObj->getSpiId(), NULL, 0, &buff, 0, 1) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"),"Error while transmitting data");
        return 0;
    }
    return buff;
}

jvoid nativeSpiWrite(FNIEnv *env, jobject obj, jint b) {
    uint8_t buff = b;
    SpiObject spiObj = (SpiObject)obj;
    if(!checkSpiTransferCondition(env, spiObj)) return;
    if(NativeSPI_Transfer(spiObj->getSpiId(), &buff, 0, NULL, 0, 1) != ESP_OK)
        env->throwNew(env->findClass("java/io/IOException"),"Error while transmitting data");
}

jint nativeSpiReadWrite(FNIEnv *env, jobject obj, jbyteArray tx, jint txOff, jboolArray rx, jint rxOff, jint length) {
    SpiObject spiObj = (SpiObject)obj;
    if(!checkSpiTransferCondition(env, spiObj)) return 0;
    if((tx == NULL) && (rx == NULL)) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return 0;
    }
    if(!checkSpiInputParam(env, tx, txOff, length)) return 0;
    if(!checkSpiInputParam(env, rx, rxOff, length)) return 0;
    uint8_t *txBuff = tx != NULL ? (uint8_t *)tx->getData() : NULL;
    uint8_t *rxBuff = rx != NULL ? (uint8_t *)rx->getData() : NULL;
    if(NativeSPI_Transfer(spiObj->getSpiId(), txBuff, txOff, rxBuff, rxOff, length) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"),"Error while transmitting data");
        return 0;
    }
    return rxBuff ? length : 0;
}

jvoid nativeSpiClose(FNIEnv *env, jobject obj) {
    SpiObject spiObj = (SpiObject)obj;
    int32_t spiId = spiObj->getSpiId();
    NativeSPI_Close(spiId);
}
