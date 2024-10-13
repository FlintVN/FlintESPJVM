
#include <string.h>
#include "flint.h"
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_system_api.h"
#include "esp_system_native_port.h"

#if CONFIG_IDF_TARGET_ESP32
static bool checkPin(FlintExecution &execution, FlintObject *pinsObj, uint32_t arrayLength) {
    uint8_t *pins = pinsObj->data;
    for(uint8_t i = 0; i < arrayLength; i++) {
        uint8_t pin = pins[i];
        if((pin == 1) || (pin == 3)) {
            const char *msg[] = {"Pin number ", (pin == 1) ? "1" : "3", " is used for debugger, you cannot use this pin"};
            FlintString &strObj = execution.flint.newString(msg, LENGTH(msg));
            FlintThrowable &excpObj = execution.flint.newErrorException(strObj);
            execution.stackPushObject(&excpObj);
            return false;
        }
        else if((6 <= pin) && (pin <= 11)) {
            FlintString &strObj = execution.flint.newString(STR_AND_SIZE("Pins from 6 to 11 are used for debugger, You cannot use these pins"));
            FlintThrowable &excpObj = execution.flint.newErrorException(strObj);
            execution.stackPushObject(&excpObj);
            return false;
        }
    }
    return true;
}
#else
static bool checkPin(FlintExecution &execution, FlintObject *pinsObj, uint32_t arrayLength) {
    // TODO
    return true;
}
#endif

static bool checkParam(FlintExecution &execution, FlintObject *pinsObj, uint32_t arrayLength) {
    if((pinsObj == 0) || (arrayLength < 1) || (arrayLength > 32)) {
        FlintString *strObj;
        FlintThrowable *excpObj;
        if(pinsObj == 0) {
            strObj = &execution.flint.newString(STR_AND_SIZE("pins array cannot be null object"));
            excpObj = &execution.flint.newNullPointerException(*strObj);
        }
        else {
            strObj = &execution.flint.newString(STR_AND_SIZE("The pin number must be from 1 to 32"));
            excpObj = &execution.flint.newErrorException(*strObj);
        }
        execution.stackPushObject(excpObj);
        return false;
    }
    return checkPin(execution, pinsObj, arrayLength);
}

static bool nativeSetMode(FlintExecution &execution) {
    int32_t mode = execution.stackPopInt32();
    FlintObject *pinsObj = execution.stackPopObject();
    uint32_t arrayLength = pinsObj ? (pinsObj->size / pinsObj->parseTypeSize()) : 0;

    if(!checkParam(execution, pinsObj, arrayLength))
        return false;

    uint8_t *pins = pinsObj->data;
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
    gpio_config(&io_conf);
    return true;
}

static bool nativeReadPort(FlintExecution &execution) {
    FlintObject *pinsObj = execution.stackPopObject();
    uint32_t arrayLength = pinsObj ? (pinsObj->size / pinsObj->parseTypeSize()) : 0;

    uint8_t *pins = pinsObj->data;
    uint32_t value = 0;
    #if GPIO_IN1_REG
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
    execution.stackPushInt32(value);

    return true;
}

static bool nativeWritePort(FlintExecution &execution) {
    uint32_t value = execution.stackPopInt32();
    FlintObject *pinsObj = execution.stackPopObject();
    uint32_t arrayLength = pinsObj ? (pinsObj->size / pinsObj->parseTypeSize()) : 0;

    uint8_t *pins = pinsObj->data;
    #if GPIO_OUT1_W1TC_REG
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

    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x07\x00\xD2\x5C""setMode",   "\x06\x00\xC6\x01""([BI)V", nativeSetMode),
    NATIVE_METHOD("\x08\x00\x70\x2C""readPort",  "\x05\x00\xD6\xAF""([B)I",  nativeReadPort),
    NATIVE_METHOD("\x09\x00\x8A\x20""writePort", "\x06\x00\xC6\x01""([BI)V", nativeWritePort),
};

const FlintNativeClass PORT_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x11\x00\x12\x81""machine/gpio/Port", methods);
