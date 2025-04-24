
#include <string.h>
#include "flint.h"
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_system_api.h"
#include "esp_system_native_pin.h"
#include "flint_throw_support.h"

const char *NativePin_CheckPin(int32_t pin) {
    if((1ULL << pin) & ~SOC_GPIO_VALID_GPIO_MASK)
        return "Invalid pin";
#if CONFIG_IDF_TARGET_ESP32
    else if((pin == 1) || (pin == 3))
        return "Pin 1 and 3 have been used for debugging";
    else if((6 <= pin) && (pin <= 11))
        return "You cannot use these pins from 6 to 11";
#elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
    else if(pin == 0)
        return "Pin 0 was used to select USB mode";
    else if((pin == 19) || (pin == 20))
        return "Pin 19 and 20 have been used for debugging (USB CDC)";
#endif
    return NULL;
}

void NativePin_Reset(Flint &flint) {
    ((void)flint);
    for(uint8_t i = 0; (SOC_GPIO_VALID_GPIO_MASK >> i); i++) {
        if(NativePin_CheckPin(i) == NULL)
            gpio_reset_pin((gpio_num_t)i);
    }
}

static FlintError checkPin(FlintExecution &execution, int32_t pin) {
    const char *msg = NativePin_CheckPin(pin);
    if(msg)
        return throwIOException(execution, msg);
    return ERR_OK;
}

static FlintError nativeSetMode(FlintExecution &execution) {
    int32_t mode = execution.stackPopInt32();
    int32_t pin = execution.stackPopInt32();

    RETURN_IF_ERR(checkPin(execution, pin));

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
        return throwIOException(execution, "Error while configuring the pin");
    return ERR_OK;
}

static FlintError nativeReadPin(FlintExecution &execution) {
    int32_t pin = execution.stackPopInt32();

    #ifdef GPIO_IN1_REG
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
    return ERR_OK;
}

static FlintError nativeWritePin(FlintExecution &execution) {
    int32_t level = execution.stackPopInt32();
    int32_t pin = execution.stackPopInt32();

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
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x07\x00\xD2\x5C""setMode",  "\x05\x00\xE3\xDD""(II)V", nativeSetMode),
    NATIVE_METHOD("\x07\x00\xB2\x58""readPin",  "\x04\x00\xB8\x06""(I)Z",  nativeReadPin),
    NATIVE_METHOD("\x08\x00\xA3\x62""writePin", "\x05\x00\x12\x18""(IZ)V", nativeWritePin),
};

const FlintNativeClass PIN_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x0F\x00\x74\x10""esp/machine/Pin", methods);
