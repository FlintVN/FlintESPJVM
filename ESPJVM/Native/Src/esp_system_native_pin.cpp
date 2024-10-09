
#include <string.h>
#include "flint.h"
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_system_api.h"
#include "esp_system_native_pin.h"

static bool nativeSetMode(FlintExecution &execution) {
    int32_t mode = execution.stackPopInt32();
    int32_t pin = execution.stackPopInt32();

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
    gpio_config(&io_conf);
    return true;
}

static bool nativeReadPin(FlintExecution &execution) {
    int32_t pin = execution.stackPopInt32();

    #if CONFIG_IDF_TARGET_ESP32
    {
        execution.stackPushInt32(REG_READ(GPIO_IN_REG) & (1 << pin) ? 1 : 0);
    }
    #elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
    {
        if(pin < 32)
            execution.stackPushInt32(REG_READ(GPIO_IN_REG) & (1 << pin) ? 1 : 0);
        else
            execution.stackPushInt32(REG_READ(GPIO_IN1_REG) & (1 << (pin - 32)) ? 1 : 0);
    }
    #endif

    return true;
}

static bool nativeWritePin(FlintExecution &execution) {
    int32_t level = execution.stackPopInt32();
    int32_t pin = execution.stackPopInt32();

    #if CONFIG_IDF_TARGET_ESP32
    {
        if(level)
            REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);
        else
            REG_WRITE(GPIO_OUT_W1TC_REG, 1 << pin);
    }
    #elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
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
    #endif

    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x07\x00\xD1\x02""setMode",  "\x05\x00\x39\x01""(II)V", nativeSetMode),
    NATIVE_METHOD("\x07\x00\xC3\x02""readPin",  "\x04\x00\xF4\x00""(I)Z",  nativeReadPin),
    NATIVE_METHOD("\x08\x00\x52\x03""writePin", "\x05\x00\x4A\x01""(IZ)V", nativeWritePin),
};

const FlintNativeClass PIN_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x10\x00\x09\x06""machine/gpio/Pin", methods);
