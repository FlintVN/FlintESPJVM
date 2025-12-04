
#include <string.h>
#include "flint.h"
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_system_api.h"
#include "esp_native_pin.h"
#include "esp_native_port.h"

static bool CheckPin(FNIEnv *env, jbyteArray pinsObj, uint32_t arrayLength) {
    uint8_t *pins = (uint8_t *)pinsObj->getData();
    for(uint8_t i = 0; i < arrayLength; i++) {
        const char *msg = NativePin_CheckPin(pins[i]);
        if(msg) {
            env->throwNew(env->findClass("java/io/IOException"), msg);
            return false;
        }
    }
    return true;
}

static bool CheckParams(FNIEnv *env, jbyteArray pinsObj, uint32_t arrayLength) {
    if((pinsObj == NULL) || (arrayLength < 1) || (arrayLength > 32)) {
        if(pinsObj == NULL) {
            env->throwNew(env->findClass("java/lang/NullPointerException"), "pins array cannot be null object");
            return false;
        }
        else {
            env->throwNew(env->findClass("java/io/IOException"), "The pin number must be from 1 to 32");
            return false;
        }
    }
    return CheckPin(env, pinsObj, arrayLength);
}

jvoid NativePort_SetMode(FNIEnv *env, jbyteArray pinsObj, jint mode) {
    uint32_t arrayLength = pinsObj ? pinsObj->getLength() : 0;

    if(!CheckParams(env, pinsObj, arrayLength)) return;

    uint8_t *pins = (uint8_t *)pinsObj->getData();
    uint64_t pinMask = 0;
    for(uint8_t i = 0; i < arrayLength; i++)
        pinMask |= (uint64_t)1 << pins[i];

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = pinMask;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    switch(mode) {
        case 0: /* INPUT */
            io_conf.mode = GPIO_MODE_INPUT;
            break;
        case 1: /* OUTPUT */
            io_conf.mode = GPIO_MODE_OUTPUT;
            break;
        case 2: /* INPUT_PULL_UP */
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            break;
        case 3: /* INPUT_PULL_DOWN */
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        case 4: /* OUTPUT_OPEN_DRAIN */
            io_conf.mode = GPIO_MODE_OUTPUT_OD;
            break;
        default:
            io_conf.mode = GPIO_MODE_DISABLE;
            break;
    }
    if(gpio_config(&io_conf) != ESP_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Error while configuring the pin");
}

jint NativePort_Read(FNIEnv *env, jobject obj) {
    jbyteArray pinsObj = (jbyteArray)obj->getFieldByIndex(0)->getObj();
    if(pinsObj == NULL) return 0;
    uint32_t arrayLength = pinsObj->getLength();
    uint8_t *pins = (uint8_t *)pinsObj->getData();
    uint32_t value = 0;
    #ifdef GPIO_IN1_REG
    {
        for(uint8_t i = 0; i < arrayLength; i++) {
            if(pins[i] < 32)
                value |= ((REG_READ(GPIO_IN_REG) & (1 << pins[i])) ? 1 : 0) << i;
            else
                value |= ((REG_READ(GPIO_IN1_REG) & (1 << (pins[i] - 32))) ? 1 : 0) << i;
        }
    }
    #else
    {
        for(uint8_t i = 0; i < arrayLength; i++)
            value |= ((REG_READ(GPIO_IN_REG) & (1 << pins[i])) ? 1 : 0) << i;
    }
    #endif
    return value;
}

jvoid NativePort_Write(FNIEnv *env, jobject obj, jint value) {
    JInt8Array *pinsObj = (JInt8Array *)obj->getFieldByIndex(0)->getObj();
    if(pinsObj == NULL) return;
    uint32_t arrayLength = pinsObj->getLength();
    uint8_t *pins = (uint8_t *)pinsObj->getData();
    #ifdef GPIO_OUT1_W1TC_REG
    {
        uint64_t setMask = 0;
        uint64_t clearMask = 0;
        for(uint8_t i = 0; i < arrayLength; i++) {
            if(value & (1 << i))
                setMask |= (uint64_t)1 << pins[i];
            else
                clearMask |= (uint64_t)1 << pins[i];
        }
        REG_WRITE(GPIO_OUT_W1TC_REG, (uint32_t)clearMask);
        REG_WRITE(GPIO_OUT1_W1TC_REG, (uint32_t)(clearMask >> 32));
        REG_WRITE(GPIO_OUT_W1TS_REG, (uint32_t)setMask);
        REG_WRITE(GPIO_OUT1_W1TS_REG, (uint32_t)(setMask >> 32));
    }
    #else
    {
        uint32_t setMask = 0;
        uint32_t clearMask = 0;
        for(uint8_t i = 0; i < arrayLength; i++) {
            if(value & (1 << i))
                setMask |= 1 << pins[i];
            else
                clearMask |= 1 << pins[i];
        }
        REG_WRITE(GPIO_OUT_W1TC_REG, clearMask);
        REG_WRITE(GPIO_OUT_W1TS_REG, setMask);
    }
    #endif
}

jvoid NativePort_Reset(FNIEnv *env, jobject obj) {
    jbyteArray pinsObj = (jbyteArray)obj->getFieldByIndex(0)->getObj();
    if(!pinsObj) return;
    uint32_t arrayLength = pinsObj->getLength();
    uint8_t *pins = (uint8_t *)pinsObj->getData();
    #ifdef GPIO_OUT1_W1TC_REG
    {
        uint64_t clearMask = 0;
        for(uint8_t i = 0; i < arrayLength; i++)
            clearMask |= (uint64_t)1 << pins[i];
        REG_WRITE(GPIO_OUT_W1TC_REG, (uint32_t)clearMask);
        REG_WRITE(GPIO_OUT1_W1TC_REG, (uint32_t)(clearMask >> 32));
    }
    #else
    {
        uint32_t clearMask = 0;
        for(uint8_t i = 0; i < arrayLength; i++)
            clearMask |= 1 << pins[i];
        REG_WRITE(GPIO_OUT_W1TC_REG, clearMask);
    }
    #endif
}
