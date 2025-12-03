
#include <string.h>
#include "sdkconfig.h"
#include "esp_adc/adc_oneshot.h"
#include "flint.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "esp_native_adc.h"

static adc_oneshot_unit_handle_t adcHandle[2] = {};

void NativeAdc_Reset(void) {
    if(adcHandle[0] != NULL) {
        adc_oneshot_del_unit(adcHandle[0]);
        adcHandle[0] = NULL;
    }
    if(adcHandle[1] != NULL) {
        adc_oneshot_del_unit(adcHandle[1]);
        adcHandle[1] = NULL;
    }
}

static int32_t getAdcId(FNIEnv *env, jstring name) {
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

jvoid nativeAdcInitAdc(FNIEnv *env, jobject obj) {
    jstring name = (jstring)obj->getFieldByIndex(0)->getObj();
    int32_t id = getAdcId(env, name);
    esp_err_t err = ESP_OK;
    if(id == -1) return;

    if(obj->getFieldByIndex(2)->getInt32() < 0)
        obj->getFieldByIndex(2)->setInt32(0);

    if(adcHandle[id] == NULL) {
        adc_oneshot_unit_init_cfg_t initCfg = {};
        initCfg.unit_id = (adc_unit_t)id;
        initCfg.ulp_mode = ADC_ULP_MODE_DISABLE;
#if SOC_ADC_DIG_CTRL_SUPPORTED && !SOC_ADC_RTC_CTRL_SUPPORTED
        initCfg.clk_src = ADC_DIGI_CLK_SRC_DEFAULT;
#elif SOC_ADC_RTC_CTRL_SUPPORTED
        initCfg.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
#endif
        err = adc_oneshot_new_unit(&initCfg, &adcHandle[id]);
    }
    if(err == ESP_OK) {
        adc_oneshot_chan_cfg_t cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        int32_t channel = obj->getFieldByIndex(2)->getInt32();
        err = adc_oneshot_config_channel(adcHandle[id], (adc_channel_t)channel, &cfg);
    }
    if(err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while initializing %.*s", name->getLength(), name->getAscii());
        return;
    }

    obj->getFieldByIndex(1)->setInt32(id);
}

jint nativeAdcRead(FNIEnv *env, jobject obj) {
    int32_t id = obj->getFieldByIndex(1)->getInt32();
    int32_t channel = obj->getFieldByIndex(2)->getInt32();
    int val;
    if(adc_oneshot_read(adcHandle[id], (adc_channel_t)channel, &val) != ESP_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Error while reading ADC%d channel %d", id + 1, channel);
    return val;
}
