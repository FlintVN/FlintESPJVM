
#include <string.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "flint.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "esp_native_dac.h"

#if SOC_DAC_SUPPORTED

#include "driver/dac_oneshot.h"

static dac_oneshot_handle_t dacHandle[2] = {};

void NativeDac_Reset(void) {
    if(dacHandle[0] != NULL) {
        dac_oneshot_del_channel(dacHandle[0]);
        dacHandle[0] = NULL;
    }
    if(dacHandle[1] != NULL) {
        dac_oneshot_del_channel(dacHandle[1]);
        dacHandle[1] = NULL;
    }
}

static int32_t GetDacId(FNIEnv *env, jstring name) {
    if(name == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "DAC name cannot be null");
        return -1;
    }
    if(name->compareTo("DAC1", strlen("DAC1")) == 0)
        return DAC_CHAN_0;
    else if(name->compareTo("DAC2", strlen("DAC2")) == 0)
        return DAC_CHAN_1;
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid DAC name");
        return -1;
    }
}

jvoid NativeDac_InitDac(FNIEnv *env, jobject obj) {
    jstring name = (jstring)obj->getFieldByIndex(0)->getObj();
    int32_t id = GetDacId(env, name);
    if(id < 0) return;
    dac_oneshot_config_t channelCfg = {.chan_id = (dac_channel_t)id};
    if(dac_oneshot_new_channel(&channelCfg, &dacHandle[id]) != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while initializing %.*s", name->getLength(), name->getAscii());
        return;
    }
    obj->getFieldByIndex(1)->setInt32(id);
}

jvoid NativeDac_Write(FNIEnv *env, jobject obj, jint value) {
    int32_t id = obj->getFieldByIndex(1)->getInt32();
    if(dac_oneshot_output_voltage(dacHandle[id], value) != ESP_OK) {
        jstring name = (jstring)obj->getFieldByIndex(0)->getObj();
        env->throwNew(env->findClass("java/io/IOException"), "Error while writing to %.*s", name->getLength(), name->getAscii());
    }
}

#else

void NativeDac_Reset(void) {

}

jvoid NativeDac_InitDac(FNIEnv *env, jobject obj) {
    (void)obj;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "DAC not supported");
}

jvoid NativeDac_Write(FNIEnv *env, jobject obj, jint value) {
    (void)obj;
    (void)value;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "DAC not supported");
}

#endif
