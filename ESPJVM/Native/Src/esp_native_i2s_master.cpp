
#include <string.h>
#include "flint.h"
#include "driver/i2s_std.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "flint_system_api.h"
#include "esp_native_common.h"
#include "esp_native_pin.h"
#include "esp_native_i2s_master.h"

#define BUF_LEN_IN_I2S_FRAMES   256

typedef class : public JObject {
public:
    jstring getI2sName() { return (jstring)getFieldByIndex(0)->getObj(); }
    jint GetI2sId() { return getFieldByIndex(1)->getInt32(); }
    jint getMode() { return getFieldByIndex(2)->getInt32(); }
    jint getDataMode() { return getMode() & 0x01; }
    jint getDirection() { return (getMode() >> 1) & 0x01; }
    jint getDataBits() { return (getMode() >> 2) & 0x3F; }
    jint getSampleRate() { return getFieldByIndex(3)->getInt32(); }
    jint getBufferSize() { return getFieldByIndex(4)->getInt32(); }
    jint getSck() { return getFieldByIndex(5)->getInt32(); }
    jint getWs() { return getFieldByIndex(6)->getInt32(); }
    jint getSd() { return getFieldByIndex(7)->getInt32(); }
    jint getMclk() { return getFieldByIndex(8)->getInt32(); }

    void setI2sId(int32_t val) { getFieldByIndex(1)->setInt32(val); }
    void setBufferSize(int32_t val) { getFieldByIndex(4)->setInt32(val); }
    void setSck(int32_t val) { getFieldByIndex(5)->setInt32(val); }
    void setWs(int32_t val) { getFieldByIndex(6)->setInt32(val); }
    void setSd(int32_t val) { getFieldByIndex(7)->setInt32(val); }
    void setMclk(int32_t val) { getFieldByIndex(8)->setInt32(val); }
} *I2sMasterObject;

typedef struct {
    i2s_chan_handle_t handle;
    I2sMasterObject i2sObj;
} I2sHandle;

static I2sHandle i2sHandle[SOC_I2S_NUM] = {};

static bool NativeI2sMaster_IsOpen(int32_t i2sId) {
    if(0 <= i2sId || i2sId < LENGTH(i2sHandle))
        return i2sHandle[i2sId].i2sObj != NULL ? true : false;
    return false;
}

static bool NativeI2sMaster_Write(FNIEnv *env, int32_t i2sId, int8_t *buff, uint32_t count) {
    size_t bw;
    i2s_chan_handle_t handle = i2sHandle[i2sId].handle;
    while(1) {
        if(i2s_channel_write(handle, buff, count, &bw, portMAX_DELAY) != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing");
            return false;
        }
        if(bw == count)
            return true;
        else {
            if(env->exec->hasTerminateRequest())
                return false;
            count -= bw;
            buff += bw;
        }
    }
}

static int32_t NativeI2sMaster_Read(FNIEnv *env, int32_t i2sId, int8_t *buff, uint32_t count) {
    size_t br;
    if(i2s_channel_read(i2sHandle[i2sId].handle, buff, count, &br, portMAX_DELAY) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while reading");
        return false;
    }
    return br;
}

static void NativeI2sMaster_Close(int32_t i2sId) {
    if(i2sHandle[i2sId].handle != NULL) {
        i2s_channel_disable(i2sHandle[i2sId].handle);
        i2s_del_channel(i2sHandle[i2sId].handle);
    }
    i2sHandle[i2sId].handle = NULL;
    i2sHandle[i2sId].i2sObj = NULL;
}

void NativeI2sMaster_Reset(void) {
    for(int8_t i = 0; i < LENGTH(i2sHandle); i++)
        NativeI2sMaster_Close(i);
}

static int32_t GetI2sId(FNIEnv *env, jstring i2sName) {
    if(i2sName == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "I2S name cannot be null");
        return -1;
    }
    if(i2sName->compareTo("I2S0", strlen("I2S0")) == 0)
        return I2S_NUM_0;
#if SOC_I2S_NUM > 1
    else if(i2sName->compareTo("I2S1", strlen("I2S1")) == 0)
        return I2S_NUM_1;
#endif
#if SOC_I2S_NUM > 2
    else if(i2sName->compareTo("I2S2", strlen("I2S2")) == 0)
        return I2S_NUM_2;
#endif
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid I2S name");
        return -1;
    }
}

static bool CheckI2sMasterPin(FNIEnv *env, I2sMasterObject i2sObj) {
    const char *msg;
    int32_t sck = i2sObj->getSck();
    int32_t ws = i2sObj->getWs();
    int32_t sd = i2sObj->getSd();
    int32_t mclk = i2sObj->getMclk();

    if(sck < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "SCK pin must be specified");
        return false;
    }
    if(ws < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "WS pin must be specified");
        return false;
    }
    if(sd < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "SD pin must be specified");
        return false;
    }
    if(
        (msg = NativePin_CheckPin(sck)) != NULL ||
        (msg = NativePin_CheckPin(ws)) != NULL ||
        (msg = NativePin_CheckPin(sd)) != NULL ||
        (mclk >= 0 && (msg = NativePin_CheckPin(sd)) != NULL)
    ) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    return true;
}

static bool CheckPrecondition(FNIEnv *env, I2sMasterObject i2sObj) {
    int32_t i2sId = i2sObj->GetI2sId();
    if(!NativeI2sMaster_IsOpen(i2sId)) {
        env->throwNew(env->findClass("java/io/IOException"), "I2S has not been opened yet");
        return false;
    }
    else if(i2sHandle[i2sId].i2sObj != i2sObj) {
        env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
        return false;
    }
    return true;
}

jobject NativeI2sMaster_Open(FNIEnv *env, jobject obj) {
    I2sMasterObject i2sObj = (I2sMasterObject)obj;
    int32_t i2sId = GetI2sId(env, i2sObj->getI2sName());
    uint32_t minBuff = (i2sObj->getDataBits() / 8 * (i2sObj->getDataMode() != 0 ? 2 : 1)) * BUF_LEN_IN_I2S_FRAMES * 2;
    if(i2sId == -1) return obj;

    if(i2sObj->getSck() < 0) i2sObj->setSck(-1);
    if(i2sObj->getWs() < 0) i2sObj->setWs(-1);
    if(i2sObj->getSd() < 0) i2sObj->setSd(-1);
    if(i2sObj->getMclk() < 0) i2sObj->setMclk(-1);
    if(i2sObj->getBufferSize() < 0) i2sObj->setBufferSize(2048 > minBuff ? 2048 : minBuff);

    if(i2sObj->getBufferSize() < minBuff) {
        env->throwNew(env->findClass("java/io/IOException"), "The minimum required buffer size is %d", minBuff);
        return obj;
    }
    if(NativeI2sMaster_IsOpen(i2sId)) {
        const char *msg = (i2sHandle[i2sId].i2sObj != i2sObj) ? "Access is denied" : "I2S is already open";
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return obj;
    }
    if(!CheckI2sMasterPin(env, i2sObj)) return obj;

    i2s_chan_config_t channelCfg = I2S_CHANNEL_DEFAULT_CONFIG((i2s_port_t)i2sId, I2S_ROLE_MASTER);
    channelCfg.dma_desc_num = i2sObj->getBufferSize() / BUF_LEN_IN_I2S_FRAMES / (i2sObj->getDataBits() / 8 * (i2sObj->getDataMode() != 0 ? 2 : 1));
    channelCfg.dma_frame_num = BUF_LEN_IN_I2S_FRAMES;
    esp_err_t err = i2s_new_channel(&channelCfg, &i2sHandle[i2sId].handle, NULL);
    if(err == ESP_OK) {
        i2s_data_bit_width_t dataBits;
        switch(i2sObj->getDataBits()) {
            case 8:
                dataBits = I2S_DATA_BIT_WIDTH_8BIT;
                break;
            case 16:
                dataBits = I2S_DATA_BIT_WIDTH_16BIT;
                break;
            case 24:
                dataBits = I2S_DATA_BIT_WIDTH_24BIT;
            default:
                dataBits = I2S_DATA_BIT_WIDTH_32BIT;

        }
        i2s_slot_mode_t mode = i2sObj->getDataMode() == 0 ? I2S_SLOT_MODE_MONO : I2S_SLOT_MODE_STEREO;
        i2s_std_config_t i2sStdCfg = {
            .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG((uint32_t)i2sObj->getSampleRate()),
            .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(dataBits, mode),
            .gpio_cfg = {
                .mclk = (gpio_num_t)i2sObj->getMclk(),
                .bclk = (gpio_num_t)i2sObj->getSck(),
                .ws   = (gpio_num_t)i2sObj->getWs(),
                .dout = (gpio_num_t)(i2sObj->getDirection() == 0 ? i2sObj->getSd() : -1),
                .din  = (gpio_num_t)(i2sObj->getDirection() == 1 ? i2sObj->getSd() : -1),
                .invert_flags = {
                    .mclk_inv = false,
                    .bclk_inv = false,
                    .ws_inv   = false,
                },
            },
        };
        err = i2s_channel_init_std_mode(i2sHandle[i2sId].handle, &i2sStdCfg);
    }
    if(err == ESP_OK) err = i2s_channel_enable(i2sHandle[i2sId].handle);
    if(err != ESP_OK) {
        NativeI2sMaster_Close(i2sId);
        env->throwNew(env->findClass("java/io/IOException"), "Error while opening I2S port");
        return obj;
    }
    i2sHandle[i2sId].i2sObj = i2sObj;
    i2sObj->setI2sId(i2sId);
    return obj;
}

jbool NativeI2sMaster_IsOpen(FNIEnv *env, jobject obj) {
    I2sMasterObject i2sObj = (I2sMasterObject)obj;
    int32_t i2sId = i2sObj->GetI2sId();
    if(NativeI2sMaster_IsOpen(i2sId))
        return (i2sHandle[i2sId].i2sObj == i2sObj) ? true : false;
    return false;
}

jint NativeI2sMaster_ReadByte(FNIEnv *env, jobject obj) {
    I2sMasterObject i2sObj = (I2sMasterObject)obj;
    if(i2sObj->getDirection() != 1) {
        env->throwNew(env->findClass("java/io/IOException"), "Write operation is not available in TX mode");
        return -1;
    }
    int8_t buff;
    int32_t i2sId = i2sObj->GetI2sId();
    if(!CheckPrecondition(env, i2sObj)) return -1;
    NativeI2sMaster_Read(env, i2sId, &buff, 1);
    return buff;
}

jint NativeI2sMaster_Read(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {
    I2sMasterObject i2sObj = (I2sMasterObject)obj;
    if(i2sObj->getDirection() != 1) {
        env->throwNew(env->findClass("java/io/IOException"), "Write operation is not available in TX mode");
        return 0;
    }
    int32_t i2sId = i2sObj->GetI2sId();
    if(!CheckPrecondition(env, i2sObj)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;
    return NativeI2sMaster_Read(env, i2sId, &b->getData()[off], count);
}

jvoid NativeI2sMaster_WriteByte(FNIEnv *env, jobject obj, jint b) {
    I2sMasterObject i2sObj = (I2sMasterObject)obj;
    if(i2sObj->getDirection() != 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Write operation is not available in RX mode");
        return;
    }
    int8_t buff = (uint8_t)b;
    int32_t i2sId = i2sObj->GetI2sId();
    if(!CheckPrecondition(env, i2sObj)) return;
    NativeI2sMaster_Write(env, i2sId, &buff, 1);
}

jvoid NativeI2sMaster_Write(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {
    I2sMasterObject i2sObj = (I2sMasterObject)obj;
    if(i2sObj->getDirection() != 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Write operation is not available in RX mode");
        return;
    }
    int32_t i2sId = i2sObj->GetI2sId();
    if(!CheckPrecondition(env, i2sObj)) return;
    if(!CheckArrayIndexSize(env, b, off, count)) return;
    NativeI2sMaster_Write(env, i2sId, &b->getData()[off], count);
}

jvoid NativeI2sMaster_Close(FNIEnv *env, jobject obj) {
    I2sMasterObject i2sObj = (I2sMasterObject)obj;
    int32_t i2sId = i2sObj->GetI2sId();
    if(NativeI2sMaster_IsOpen(i2sId)) {
        if(i2sHandle[i2sId].i2sObj != i2sObj) {
            env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
            return;
        }
        NativeI2sMaster_Close(i2sId);
    }
    i2sObj->setI2sId(-1);
}
