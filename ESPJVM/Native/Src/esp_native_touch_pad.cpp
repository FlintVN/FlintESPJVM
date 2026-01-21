
#include <string.h>
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "flint.h"
#include "flint_java_object.h"
#include "esp_native_pin.h"
#include "esp_native_touch_pad.h"

#if SOC_TOUCH_SENSOR_SUPPORTED

#include "driver/touch_pad.h"

#if CONFIG_IDF_TARGET_ESP32
static const int8_t touchPin[] = {4, 0, 2, 15, 13, 12, 14, 27, 33, 32};
#elif CONFIG_IDF_TARGET_ESP32S2
static const int8_t touchPin[] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
#elif CONFIG_IDF_TARGET_ESP32S3
static const int8_t touchPin[] = {-1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
#endif

static uint8_t initialized = 0;

void NativeTouchPad_Reset(void) {
    if(initialized != 0) {
        initialized = 0;
        touch_pad_deinit();
    }
}

static int32_t NativeTouchPad_GetPadNum(FNIEnv *env, int32_t pin) {
    if(pin >= 0) {
        const char *msg = NativePin_CheckPin(pin);
        if(msg != NULL) {
            env->throwNew(env->findClass("java/io/IOException"), msg);
            return -1;
        }
        for(uint8_t i = 0; i < LENGTH(touchPin); i++) {
            if(touchPin[i] == pin)
                return i;
        }
    }
    env->throwNew(env->findClass("java/io/IOException"), "Invalid TouchPad pin");
    return -1;
}

static esp_err_t NativeTouchPad_DriverInit(void) {
    esp_err_t err = touch_pad_init();
    if(err == ESP_OK) {
        err = touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
#if SOC_TOUCH_SENSOR_VERSION == 1
        if(err == ESP_OK) err = touch_pad_filter_start(10);
#elif SOC_TOUCH_SENSOR_VERSION == 2
        if(err == ESP_OK) err = touch_pad_fsm_start();
#endif
        if(err != ESP_OK)
            touch_pad_deinit();
    }
    return err;
}

static int32_t NativeTouchPad_Read(FNIEnv *env, int32_t pad) {
#if SOC_TOUCH_SENSOR_VERSION == 1
    uint16_t val;
    esp_err_t err = touch_pad_read_raw_data((touch_pad_t)pad, &val);
    if(err != ESP_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Error while Reading TouchPad");
    return 0xFFFF - val;
#elif SOC_TOUCH_SENSOR_VERSION == 2
    uint32_t val;
    esp_err_t err = touch_pad_read_raw_data((touch_pad_t)pad, &val);
    if(err != ESP_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Error while Reading TouchPad");
    return val;
#else
    #error "Touch sensor version is not supported"
#endif
}

jint NativeTouchPad_InitTouchPad(FNIEnv *env, jobject obj, jint pin) {
    int32_t pad = NativeTouchPad_GetPadNum(env, pin);
    if(pad == -1) return -1;
    esp_err_t err = ESP_OK;
    if(initialized == 0) {
        Flint::lock();
        if(initialized == 0) {
            err = NativeTouchPad_DriverInit();
            if(err == ESP_OK)
                initialized = 1;
        }
        Flint::unlock();
    }
#if SOC_TOUCH_SENSOR_VERSION == 1
    if(err == ESP_OK) err = touch_pad_config((touch_pad_t)pad, -1);
#elif SOC_TOUCH_SENSOR_VERSION == 2
    if(err == ESP_OK) err = touch_pad_config((touch_pad_t)pad);
#else
    #error "Touch sensor version is not supported"
#endif
    if(err != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while initializing TouchPad");
        return -1;
    }
    obj->getFieldByIndex(0)->setInt32(pad);
    return pad;
}

jint NativeTouchPad_Read(FNIEnv *env, jobject obj) {
    int32_t pad = obj->getFieldByIndex(0)->getInt32();
    return NativeTouchPad_Read(env, pad);
}

jbool NativeTouchPad_IsTouched(FNIEnv *env, jobject obj) {
    int32_t pad = obj->getFieldByIndex(0)->getInt32();
    int32_t threshold = obj->getFieldByIndex(1)->getInt32();
    return NativeTouchPad_Read(env, pad) > threshold ? true : false;
}

#else

void NativeTouchPad_Reset(void) {

}

jint NativeTouchPad_InitTouchPad(FNIEnv *env, jobject obj, jint pin) {
    (void)obj;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "TouchPad not supported");
    return -1;
}

jint NativeTouchPad_Read(FNIEnv *env, jobject obj) {
    return -1;
}

jbool NativeTouchPad_IsTouched(FNIEnv *env, jobject obj) {
    return false;
}

#endif /* SOC_TOUCH_SENSOR_SUPPORTED */
