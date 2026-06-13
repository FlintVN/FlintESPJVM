
#include <string.h>
#include "flint.h"
#include "soc/spi_pins.h"
#include "driver/spi_master.h"
#include "flint_system_api.h"
#include "flint_native_common.h"
#include "esp_native_pin.h"
#include "esp_native_spi_master.h"

#define MAX_TRANSFER_SZ     1024

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
} *SpiMasterObject;

static spi_device_handle_t spiHandle[SPI_HOST_MAX - 1] = {};

static bool NativeSpiMaster_IsOpen(int32_t spiId) {
    if(spiId == 1 || spiId == 2)
        return spiHandle[spiId - 1] ? true : false;
    return false;
}

static esp_err_t NativeSpiMaster_Transfer(FNIEnv *env, int32_t spiId, uint8_t *txBuff, int32_t txOff, uint8_t *rxBuff, int32_t rxOff, int32_t len) {
    spi_device_handle_t handle = spiHandle[spiId - 1];

    if(len <= 4) {
        spi_transaction_t t = {};

        if(txBuff != NULL) memcpy(&t.tx_data, &txBuff[txOff], len);

        t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
        t.length = len * 8;
        esp_err_t err = spi_device_transmit(handle, &t);
        if(err != ESP_OK) return err;

        if(rxBuff != NULL) memcpy(&rxBuff[rxOff], &t.rx_data, len);

        return err;
    }
    else {
        uint32_t offset = 0;
        spi_transaction_t *t, *r, transactions[2];

        if(txBuff != NULL) txBuff += txOff;
        if(rxBuff != NULL) rxBuff += rxOff;

        esp_err_t err = spi_device_acquire_bus(handle, portMAX_DELAY);
        if(err != ESP_OK) return err;

        while(len) {
            t = (t == &transactions[0]) ? &transactions[1] : &transactions[0];
            memset(t, 0, sizeof(spi_transaction_t));

            uint32_t s = len < MAX_TRANSFER_SZ ? len : MAX_TRANSFER_SZ;
            t->length = s * 8;

            if(txBuff != NULL) t->tx_buffer = &txBuff[offset];
            if(rxBuff != NULL) t->rx_buffer = &rxBuff[offset];

            err = spi_device_queue_trans(handle, t, portMAX_DELAY);
            if(err != ESP_OK) break;

            if(env->hasTerminateRequest()) {
                spi_device_release_bus(handle);
                return ESP_OK;
            }

            if(offset > 0) {
                err = spi_device_get_trans_result(handle, &r, portMAX_DELAY);
                if(err != ESP_OK) break;
            }

            len -= s;
            offset += s;
        }

        if(err == ESP_OK) err = spi_device_get_trans_result(handle, &r, portMAX_DELAY);
        spi_device_release_bus(handle);

        return err;
    }
}

static void NativeSpiMaster_Close(int32_t spiId) {
    if(NativeSpiMaster_IsOpen(spiId)) {
        int32_t index = spiId - 1;
        spi_bus_remove_device(spiHandle[index]);
        spi_bus_free((spi_host_device_t)spiId);
        spiHandle[index] = NULL;
    }
}

void NativeSpiMaster_Reset(void) {
    for(uint8_t i = 0; i < LENGTH(spiHandle); i++)
        NativeSpiMaster_Close(i + 1);
}

static bool CheckSpiPin(FNIEnv *env, SpiMasterObject spiObj) {
    const char *msg;
    if(spiObj->getMosi() < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "MOSI pin value cannot be -1 when in SPI Master mode");
        return false;
    }
    int32_t miso = spiObj->getMiso();
    if(miso >= 0 && (msg = NativePin_CheckPin(miso)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
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
    if(cs >= 0 && (msg = NativePin_CheckPin(cs)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    return true;
}

static bool CheckSpiTransferCondition(FNIEnv *env, SpiMasterObject spiObj) {
    int32_t spiId = spiObj->getSpiId();
    if(!NativeSpiMaster_IsOpen(spiId)) {
        env->throwNew(env->findClass("java/io/IOException"), "SPI has not been opened yet");
        return false;
    }
    return true;
}

static int32_t GetSpiId(FNIEnv *env, jstring spiName) {
    if(spiName == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "SPI name cannot be null");
        return -1;
    }
    else if(spiName->compareTo("SPI0", strlen("SPI0")) == 0 || spiName->compareTo("SPI1", strlen("SPI1")) == 0) {
        env->throwNew(env->findClass("java/io/IOException"), "SPI0/SPI1 was used for Flash/PSRAM memory");
        return -1;
    }
    else if(spiName->compareTo("SPI2", strlen("SPI2")) == 0)
        return SPI2_HOST;
#if SOC_SPI_PERIPH_NUM > 2
    else if(spiName->compareTo("SPI3", strlen("SPI3")) == 0)
        return SPI3_HOST;
#endif
#if CONFIG_IDF_TARGET_ESP32
    else if(spiName->compareTo("HSPI", strlen("HSPI")) == 0)
        return HSPI_HOST;
    else if(spiName->compareTo("VSPI", strlen("VSPI")) == 0)
        return VSPI_HOST;
#elif CONFIG_IDF_TARGET_ESP32S2
    else if(spiName->compareTo("FSPI", strlen("FSPI")) == 0)
        return FSPI_HOST;
    else if(spiName->compareTo("HSPI", strlen("HSPI")) == 0)
        return HSPI_HOST;
#endif
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid SPI name");
        return -1;
    }
}

static void InitDefaultValues(SpiMasterObject spiObj, int32_t spiId) {
#ifdef SPI3_IOMUX_PIN_NUM_MOSI
    static const int8_t spiMosiPin[] = {SPI2_IOMUX_PIN_NUM_MOSI, SPI3_IOMUX_PIN_NUM_MOSI};
    static const int8_t spiMisoPin[] = {SPI2_IOMUX_PIN_NUM_MISO, SPI3_IOMUX_PIN_NUM_MISO};
    static const int8_t spiClkPin[]  = {SPI2_IOMUX_PIN_NUM_CLK,  SPI3_IOMUX_PIN_NUM_CLK};
#else
    static const int8_t spiMosiPin[] = {SPI2_IOMUX_PIN_NUM_MOSI, -1};
    static const int8_t spiMisoPin[] = {SPI2_IOMUX_PIN_NUM_MISO, -1};
    static const int8_t spiClkPin[]  = {SPI2_IOMUX_PIN_NUM_CLK,  -1};
#endif

    if(spiObj->getSpeed() < 0) spiObj->setSpeed(5000000);
    if(spiObj->getMosi() < -1) spiObj->setMosi(spiMosiPin[spiId - 1]);
    if(spiObj->getMiso() < -1) spiObj->setMiso(spiMisoPin[spiId - 1]);
    if(spiObj->getClk() < -1) spiObj->setClk(spiClkPin[spiId - 1]);
    if(spiObj->getCs() < -1) spiObj->setCs(-1);
}

jobject NativeSpiMaster_Open(FNIEnv *env, jobject obj) {
    SpiMasterObject spiObj = (SpiMasterObject)obj;
    if(spiObj->getSpiId() > 0) {
        env->throwNew(env->findClass("java/io/IOException"), "SPI is already open");
        return obj;
    }
    int32_t spiId = GetSpiId(env, spiObj->getSpiName());
    if(spiId == -1) return obj;

    InitDefaultValues(spiObj, spiId);

    if(NativeSpiMaster_IsOpen(spiId)) {
        env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
        return obj;
    }

    if(!CheckSpiPin(env, spiObj)) return obj;

    int32_t mode = spiObj->getMode();

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = spiObj->getMosi();
    buscfg.miso_io_num = spiObj->getMiso();
    buscfg.sclk_io_num = spiObj->getClk();
    buscfg.quadwp_io_num = -1;
    buscfg.quadhd_io_num = -1;
    buscfg.max_transfer_sz = MAX_TRANSFER_SZ;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz = spiObj->getSpeed();
    devcfg.mode = (mode & 0x03);
    devcfg.spics_io_num = spiObj->getCs();
    devcfg.queue_size = 1;
    devcfg.flags |= (mode & 0x04) ? SPI_DEVICE_BIT_LSBFIRST : 0;
    devcfg.flags |= (mode & 0x08) ? SPI_DEVICE_POSITIVE_CS : 0;

#if CONFIG_IDF_TARGET_ESP32
    uint8_t dmaCh = (spiId == SPI2_HOST) ? 1 : 2;
#else
    uint8_t dmaCh = SPI_DMA_CH_AUTO;
#endif

    esp_err_t err = spi_bus_initialize((spi_host_device_t)spiId, &buscfg, dmaCh);
    if(err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while initializing bus with error code %d", (int32_t)err);
        return obj;
    }
    err = spi_bus_add_device((spi_host_device_t)spiId, &devcfg, &spiHandle[spiId - 1]);
    if(err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while adding device with error code %d", (int32_t)err);
        return obj;
    }

    spiObj->setSpiId(spiId);
    return obj;
}

jbool NativeSpiMaster_IsOpen(FNIEnv *env, jobject obj) {
    SpiMasterObject spiObj = (SpiMasterObject)obj;
    int32_t spiId = spiObj->getSpiId();
    return NativeSpiMaster_IsOpen(spiId);
}

jint NativeSpiMaster_GetSpeed(FNIEnv *env, jobject obj) {
    SpiMasterObject spiObj = (SpiMasterObject)obj;
    int32_t spiId = spiObj->getSpiId();
    if(NativeSpiMaster_IsOpen(spiId)) {
        int32_t ret = 0;
        esp_err_t err = spi_device_get_actual_freq(spiHandle[spiId - 1], (int *)&ret);
        if(err != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Error get actual spi speed with error code %d", (int32_t)err);
            return 0;
        }
        return ret * 1000;
    }
    return spiObj->getSpeed();
}

jint NativeSpiMaster_Read(FNIEnv *env, jobject obj) {
    uint8_t buff;
    SpiMasterObject spiObj = (SpiMasterObject)obj;
    if(!CheckSpiTransferCondition(env, spiObj)) return -1;
    esp_err_t err = NativeSpiMaster_Transfer(env, spiObj->getSpiId(), NULL, 0, &buff, 0, 1);
    if(err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while reading data with error code %d", (int32_t)err);
        return -1;
    }
    return buff;
}

jvoid NativeSpiMaster_Write(FNIEnv *env, jobject obj, jint b) {
    uint8_t buff = b;
    SpiMasterObject spiObj = (SpiMasterObject)obj;
    if(!CheckSpiTransferCondition(env, spiObj)) return;
    esp_err_t err = NativeSpiMaster_Transfer(env, spiObj->getSpiId(), &buff, 0, NULL, 0, 1);
    if(err != ESP_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Error while transmitting data with error code %d", (int32_t)err);
}

jint NativeSpiMaster_ReadWrite(FNIEnv *env, jobject obj, jbyteArray tx, jint txOff, jboolArray rx, jint rxOff, jint length) {
    SpiMasterObject spiObj = (SpiMasterObject)obj;
    if(!CheckSpiTransferCondition(env, spiObj)) return 0;
    if((tx == NULL) && (rx == NULL)) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return 0;
    }
    if(tx != NULL && !CheckArrayIndexSize(env, tx, txOff, length)) return 0;
    if(rx != NULL && !CheckArrayIndexSize(env, rx, rxOff, length)) return 0;
    uint8_t *txBuff = tx != NULL ? (uint8_t *)tx->getData() : NULL;
    uint8_t *rxBuff = rx != NULL ? (uint8_t *)rx->getData() : NULL;
    esp_err_t err = NativeSpiMaster_Transfer(env, spiObj->getSpiId(), txBuff, txOff, rxBuff, rxOff, length);
    if(err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while reading data with error code %d", (int32_t)err);
        return 0;
    }
    return rxBuff ? length : 0;
}

jvoid NativeSpiMaster_Close(FNIEnv *env, jobject obj) {
    SpiMasterObject spiObj = (SpiMasterObject)obj;
    int32_t spiId = spiObj->getSpiId();
    if(NativeSpiMaster_IsOpen(spiId))
        NativeSpiMaster_Close(spiId);
    spiObj->setSpiId(-1);
}
