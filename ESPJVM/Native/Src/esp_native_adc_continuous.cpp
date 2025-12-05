
#include <string.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_continuous.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "flint.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "esp_native_common.h"
#include "esp_native_adc_continuous.h"

#define CONV_FRAME_SIZE     128

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define ADC_GET_DATA(out_data)      (out_data.type1.data)
#else
#define ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define ADC_GET_DATA(out_data)      (out_data.type2.data)
#endif

typedef class : public JObject {
public:
    jstring getName() { return (jstring)getFieldByIndex(0)->getObj(); }
    jint getAdcId() { return getFieldByIndex(1)->getInt32(); }
    jint getChannel() { return getFieldByIndex(2)->getInt32(); }
    jint getSampleRate() { return getFieldByIndex(3)->getInt32(); }
    jint getBufferSize() { return getFieldByIndex(4)->getInt32(); }

    void setAdcId(int32_t val) { getFieldByIndex(1)->setInt32(val); }
    void setChannel(int32_t val) { getFieldByIndex(2)->setInt32(val); }
    void setSampleRate(int32_t val) { getFieldByIndex(3)->setInt32(val); }
    void setBufferSize(int32_t val) { getFieldByIndex(4)->setInt32(val); }
} *AdcContinuousObject;

static adc_continuous_handle_t handle[2] = {};

static bool NativeAdcContinuous_Stop0(int32_t id) {
    if(id < 0 || LENGTH(handle) <= id)
        return false;
    if(handle[id] != NULL) {
        esp_err_t err = adc_continuous_stop(handle[id]);
        err |= adc_continuous_deinit(handle[id]);
        handle[id] = NULL;
        return err == ESP_OK;
    }
    return true;
}

void NativeAdcContinuous_Reset(void) {
    NativeAdcContinuous_Stop0(0);
    NativeAdcContinuous_Stop0(1);
}

static int32_t GetAdcId(FNIEnv *env, jstring name) {
    if(name == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "ADC name cannot be null");
        return -1;
    }
    if(name->compareTo("ADC1", strlen("ADC1")) == 0)
        return ADC_UNIT_1;
    else if(name->compareTo("ADC2", strlen("ADC2")) == 0)
        return ADC_UNIT_2;
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid ADC name");
        return -1;
    }
}

static bool CheckPrecondition(FNIEnv *env, AdcContinuousObject adc) {
    int32_t id = adc->getAdcId();
    if(id < 0 || LENGTH(handle) <= id || handle[id] == NULL) {
        env->throwNew(env->findClass("java/io/IOException"), "AdcContinuous not started");
        return false;
    }
    return true;
}

static esp_err_t NativeAdcContinuous_Read(FNIEnv *env, int32_t id, uint16_t *b, uint32_t count) {
    static const uint32_t bufLen = CONV_FRAME_SIZE / sizeof(adc_digi_output_data_t);
    uint32_t rSize;
    adc_digi_output_data_t buf[bufLen];
    while(count > 0 && !env->exec->hasTerminateRequest()) {
        uint32_t len = (count < bufLen) ? count : bufLen;
        len *= sizeof(adc_digi_output_data_t);
        esp_err_t err = adc_continuous_read(handle[id], (uint8_t *)buf, len, &rSize, portMAX_DELAY);
        if(err != ESP_OK) {
            env->throwNew(env->findClass("java/io/IOException"), "Error while reading with error code %d", err);
            return err;
        }
        len = rSize / sizeof(adc_digi_output_data_t);
        for(uint32_t i = 0; i < len; i++)
            b[i] = ADC_GET_DATA(buf[i]);
        b += len;
        count -= len;
    }
    return ESP_OK;
}

jvoid NativeAdcContinuous_Start(FNIEnv *env, jobject obj) {
    AdcContinuousObject adc = (AdcContinuousObject)obj;
    int32_t id = GetAdcId(env, adc->getName());
    if(id == -1) return;

    if(handle[id] != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), "AdcContinuous is already init");
        return;
    }

    if(adc->getChannel() < 0) adc->setChannel(0);
    if(adc->getSampleRate() < 1) adc->setSampleRate(44100);
    if(adc->getBufferSize() < 0) adc->setBufferSize(1024);

    int32_t sampleRate = adc->getSampleRate();
    if(sampleRate < SOC_ADC_SAMPLE_FREQ_THRES_LOW || SOC_ADC_SAMPLE_FREQ_THRES_HIGH < sampleRate) {
        const char *msg = "SampleRate must be in the range of %d to %d";
        env->throwNew(env->findClass("java/io/IOException"), msg, SOC_ADC_SAMPLE_FREQ_THRES_LOW, SOC_ADC_SAMPLE_FREQ_THRES_HIGH);
        return;
    }

    int32_t buffSize = adc->getBufferSize();
    if(buffSize < CONV_FRAME_SIZE * 4) {
        env->throwNew(env->findClass("java/io/IOException"), "BufferSize must be at least 10 %d", CONV_FRAME_SIZE * 4);
        return;
    }

    adc_continuous_handle_cfg_t adcConfig = {};
    adcConfig.max_store_buf_size = (uint32_t)buffSize;
    adcConfig.conv_frame_size = CONV_FRAME_SIZE;
    esp_err_t err = adc_continuous_new_handle(&adcConfig, &handle[id]);

    if(err == ESP_OK) {
        adc_digi_pattern_config_t adcPattern = {};
        adcPattern.atten = ADC_ATTEN_DB_12;
        adcPattern.channel = adc->getChannel();
        adcPattern.unit = (adc_unit_t)id;
        adcPattern.bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

        adc_continuous_config_t digCfg = {};
        digCfg.sample_freq_hz = (uint32_t)sampleRate;
        digCfg.conv_mode = (id == 0) ? ADC_CONV_SINGLE_UNIT_1 : ADC_CONV_SINGLE_UNIT_2;
        digCfg.format = ADC_OUTPUT_TYPE;
        digCfg.pattern_num = 1;
        digCfg.adc_pattern = &adcPattern;

        err = adc_continuous_config(handle[id], &digCfg);
    }

    if(err == ESP_OK) err = adc_continuous_start(handle[id]);

    if(err != ESP_OK) {
        NativeAdcContinuous_Stop0(id);
        env->throwNew(env->findClass("java/io/IOException"), "AdcContinuous start failed with error code %d", err);
    }
    else
        adc->setAdcId(id);
}

jbool NativeAdcContinuous_IsStart(FNIEnv *env, jobject obj) {
    AdcContinuousObject adc = (AdcContinuousObject)obj;
    int32_t id = adc->getAdcId();
    return 0 <= id && id < LENGTH(handle) && handle[id] != NULL;
}

jint NativeAdcContinuous_ReadByteArray(FNIEnv *env, jobject obj, jbyteArray b, int off, int count) {
    AdcContinuousObject adc = (AdcContinuousObject)obj;
    if(!CheckPrecondition(env, adc)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;
    if(count % 2 != 0) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "The count parameter must be divisible by 2");
        return 0;
    }
    NativeAdcContinuous_Read(env, adc->getAdcId(), (uint16_t *)&b->getData()[off], count / 2);
    return count;
}

jint NativeAdcContinuous_ReadShortArray(FNIEnv *env, jobject obj, jshortArray b, int off, int count) {
    AdcContinuousObject adc = (AdcContinuousObject)obj;
    if(!CheckPrecondition(env, adc)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;
    NativeAdcContinuous_Read(env, adc->getAdcId(), (uint16_t *)&b->getData()[off], count);
    return count;
}

jint NativeAdcContinuous_ReadIntArray(FNIEnv *env, jobject obj, jintArray b, int off, int count) {
    AdcContinuousObject adc = (AdcContinuousObject)obj;
    if(!CheckPrecondition(env, adc)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;
    NativeAdcContinuous_Read(env, adc->getAdcId(), (uint16_t *)&b->getData()[off], count * 2);
    return count;
}

jvoid NativeAdcContinuous_Stop(FNIEnv *env, jobject obj) {
    AdcContinuousObject adc = (AdcContinuousObject)obj;
    int32_t id = adc->getAdcId();
    if(!NativeAdcContinuous_Stop0(id))
        env->throwNew(env->findClass("java/io/IOException"), "AdcContinuous stop error");
    adc->setAdcId(-1);
}
