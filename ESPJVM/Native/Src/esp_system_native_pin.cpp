
#include <string.h>
#include "flint.h"
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_system_api.h"
#include "esp_system_native_pin.h"

#if CONFIG_IDF_TARGET_ESP32
static bool checkPin(FlintExecution &execution, int32_t pin) {
    if((pin == 1) || (pin == 3)) {
        const char *msg[] = {"Pin number ", (pin == 1) ? "1" : "3", " is used for debugger, you cannot use this pin"};
        FlintString &strObj = execution.flint.newString(msg, LENGTH(msg));
        FlintThrowable &excpObj = execution.flint.newException(strObj);
        execution.stackPushObject(&excpObj);
        return false;
    }
    else if((6 <= pin) && (pin <= 11)) {
        FlintString &strObj = execution.flint.newString(STR_AND_SIZE("Pins from 6 to 11 are used for debugger, You cannot use these pins"));
        FlintThrowable &excpObj = execution.flint.newException(strObj);
        execution.stackPushObject(&excpObj);
        return false;
    }
    return true;
}
#else
static bool checkPin(FlintExecution &execution, int32_t pin) {
    // TODO
    return true;
}
#endif

static bool nativeSetMode(FlintExecution &execution) {
    int32_t mode = execution.stackPopInt32();
    int32_t pin = execution.stackPopInt32();

    if(!checkPin(execution, pin))
        return false;

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

    #if GPIO_IN1_REG
    {
        if(pin < 32)
            execution.stackPushInt32(REG_READ(GPIO_IN_REG) & (1 << pin) ? 1 : 0);
        else
            execution.stackPushInt32(REG_READ(GPIO_IN1_REG) & (1 << (pin - 32)) ? 1 : 0);
    }
    #else
    {
        execution.stackPushInt32(REG_READ(GPIO_IN_REG) & (1 << pin) ? 1 : 0);
    }
    #endif

    return true;
}

static bool nativeWritePin(FlintExecution &execution) {
    int32_t level = execution.stackPopInt32();
    int32_t pin = execution.stackPopInt32();

    #if GPIO_OUT_W1TC_REG
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

    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x07\x00\xD2\x5C""setMode",  "\x05\x00\xE3\xDD""(II)V", nativeSetMode),
    NATIVE_METHOD("\x07\x00\xB2\x58""readPin",  "\x04\x00\xB8\x06""(I)Z",  nativeReadPin),
    NATIVE_METHOD("\x08\x00\xA3\x62""writePin", "\x05\x00\x12\x18""(IZ)V", nativeWritePin),
};

const FlintNativeClass PIN_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x10\x00\x20\xBB""machine/gpio/Pin", methods);
