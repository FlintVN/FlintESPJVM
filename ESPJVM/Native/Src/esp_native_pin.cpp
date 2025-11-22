
#include <string.h>
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "esp_native_pin.h"

const char *NativePin_CheckPin(int32_t pin) {
    if((1ULL << pin) & ~SOC_GPIO_VALID_GPIO_MASK)
        return "Invalid pin";
#if CONFIG_IDF_TARGET_ESP32
    else if((pin == 1) || (pin == 3))
        return "Pin 1 and 3 have been used for debugging";
    else if((6 <= pin) && (pin <= 11))
        return "Pins 6 to 11 have been used for SPI Flash";
#elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
    else if(pin == 0)
        return "Pin 0 was used to select USB mode";
    else if((pin == 19) || (pin == 20))
        return "Pin 19 and 20 have been used for debugging (USB CDC)";
    else if((26 <= pin) && (pin <= 32))
        return "Pins 26 to 32 have been used for SPI Flash and PSRAM";
#elif CONFIG_IDF_TARGET_ESP32C3
    else if(pin == 18 || pin == 19)
        return "Pin 18 and 19 have been used for debugging (USB-JTAG)";
    else if((12 <= pin) && (pin <= 17))
        return "Pins 12 to 17 have been used for SPI Flash";
#elif CONFIG_IDF_TARGET_ESP32C6
    else if(pin == 12 || pin == 13)
        return "Pin 12 and 13 have been used for debugging (USB-JTAG)";
    else if(((24 <= pin) && (pin <= 26)) || ((28 <= pin) && (pin <= 30)))
        return "Pins 24, 25, 26, 28, 29 and 30 have been used for SPI Flash";
#endif
    return NULL;
}

void NativePin_Reset(void) {
    for(uint8_t i = 0; (SOC_GPIO_VALID_GPIO_MASK >> i); i++) {
        if(NativePin_CheckPin(i) == NULL)
            gpio_reset_pin((gpio_num_t)i);
    }
}

static bool checkPin(FNIEnv *env, int32_t pin) {
    const char *msg = NativePin_CheckPin(pin);
    if(msg) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    return true;
}

jvoid nativePinSetMode(FNIEnv *env, jint pin, jint mode) {
    if(!checkPin(env, pin)) return;

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (uint64_t)1 << pin;
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
        env->throwNew(env->findClass("java/io/IOException"));
}

jbool nativePinRead(FNIEnv *env, jobject obj) {
    int32_t pin = obj->getFieldByIndex(0)->getInt32();

    #ifdef GPIO_IN1_REG
    {
        if(pin < 32)
            return (REG_READ(GPIO_IN_REG) & (1 << pin)) ? true : false;
        else
            return (REG_READ(GPIO_IN1_REG) & (1 << (pin - 32))) ? true : false;
    }
    #else
    {
        return (REG_READ(GPIO_IN_REG) & (1 << pin)) ? true : false;
    }
    #endif
}

jvoid nativePinWrite(FNIEnv *env, jobject obj, jbool level) {
    int32_t pin = obj->getFieldByIndex(0)->getInt32();

    #ifdef GPIO_OUT1_W1TC_REG
    {
        if(pin < 32) {
            if(level)
                REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);
            else
                REG_WRITE(GPIO_OUT_W1TC_REG, 1 << pin);
        }
        else {
            if(level)
                REG_WRITE(GPIO_OUT1_W1TS_REG, 1 << (pin - 32));
            else
                REG_WRITE(GPIO_OUT1_W1TC_REG, 1 << (pin - 32));
        }
    }
    #else
    {
        if(level)
            REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);
        else
            REG_WRITE(GPIO_OUT_W1TC_REG, 1 << pin);
    }
    #endif
}

jvoid nativePinSet(FNIEnv *env, jobject obj) {
    int32_t pin = obj->getFieldByIndex(0)->getInt32();

    #ifdef GPIO_OUT1_W1TC_REG
    {
        if(pin < 32)
            REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);
        else
            REG_WRITE(GPIO_OUT1_W1TS_REG, 1 << (pin - 32));
    }
    #else
    {
        REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);
    }
    #endif
}

jvoid nativePinReset(FNIEnv *env, jobject obj) {
    int32_t pin = obj->getFieldByIndex(0)->getInt32();

    #ifdef GPIO_OUT1_W1TC_REG
    {
        if(pin < 32)
            REG_WRITE(GPIO_OUT_W1TC_REG, 1 << pin);
        else
            REG_WRITE(GPIO_OUT1_W1TC_REG, 1 << (pin - 32));
    }
    #else
    {
        REG_WRITE(GPIO_OUT_W1TC_REG, 1 << pin);
    }
    #endif
}

jvoid nativePinToggle(FNIEnv *env, jobject obj) {
    int32_t pin = obj->getFieldByIndex(0)->getInt32();

    #ifdef GPIO_OUT1_REG
    {
        if(pin < 32) {
            uint32_t mask = 1 << pin;
            if(REG_READ(GPIO_OUT_REG) & mask)
                REG_WRITE(GPIO_OUT_W1TC_REG, mask);
            else
                REG_WRITE(GPIO_OUT_W1TS_REG, mask);
        }
        else {
            uint32_t mask = 1 << (pin - 32);
            if(REG_READ(GPIO_OUT1_REG) & mask)
                REG_WRITE(GPIO_OUT1_W1TC_REG, mask);
            else
                REG_WRITE(GPIO_OUT1_W1TS_REG, mask);
        }
    }
    #else
    {
        uint32_t mask = 1 << pin;
        if(REG_READ(GPIO_OUT_REG) & mask)
            REG_WRITE(GPIO_OUT_W1TC_REG, mask);
        else
            REG_WRITE(GPIO_OUT_W1TS_REG, mask);
    }
    #endif
}
