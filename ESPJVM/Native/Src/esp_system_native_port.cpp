
#include <string.h>
#include "flint.h"
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_system_api.h"
#include "esp_system_native_port.h"

static bool checkParam(FlintExecution &execution, FlintObject *pinsObj, uint32_t arrayLength) {
    if((pinsObj == 0) || (arrayLength < 1) || (arrayLength > 32)) {
        FlintString *strObj;
        if(pinsObj == 0)
            strObj = &execution.flint.newString(STR_AND_SIZE("pins array cannot be null object"));
        else
            strObj = &execution.flint.newString(STR_AND_SIZE("The pin number must be from 1 to 32"));
        FlintThrowable &excpObj = execution.flint.newNullPointerException(*strObj);
        execution.stackPushObject(&excpObj);
        return false;
    }
    return true;
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

    if(!checkParam(execution, pinsObj, arrayLength))
        return false;

    uint8_t *pins = pinsObj->data;
    uint32_t value = 0;
    #if CONFIG_IDF_TARGET_ESP32
    {
        for(uint8_t i = 0; i < arrayLength; i++)
            value |= ((REG_READ(GPIO_IN_REG) & (1 << pins[i])) ? 1 : 0) << i;
    }
    #elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
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
        #error "Port.readPort method not supported for this ESP32 chip"
    }
    #endif
    execution.stackPushInt32(value);

    return true;
}

static bool nativeWritePort(FlintExecution &execution) {
    uint32_t value = execution.stackPopInt32();
    FlintObject *pinsObj = execution.stackPopObject();
    uint32_t arrayLength = pinsObj ? (pinsObj->size / pinsObj->parseTypeSize()) : 0;

    if(!checkParam(execution, pinsObj, arrayLength))
        return false;

    uint8_t *pins = pinsObj->data;
    #if CONFIG_IDF_TARGET_ESP32
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
    #elif CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
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
        #error "Port.writePort method not supported for this ESP32 chip"
    }
    #endif

    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x08\x00\x01\x03""setMode",   "\x06\x00\x8D\x01""([BI)V", nativeSetMode),
    NATIVE_METHOD("\x08\x00\x41\x03""readPort",  "\x05\x00\x37\x01""([B)I",  nativeReadPort),
    NATIVE_METHOD("\x09\x00\xD0\x03""writePort", "\x06\x00\x8D\x01""([BI)V", nativeWritePort),
};

const FlintNativeClass PORT_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x11\x00\x87\x06""machine/gpio/Port", methods);
