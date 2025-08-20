
#include <string.h>
#include "flint.h"
#include "sdkconfig.h"
#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "flint_system_api.h"
#include "esp_system_native_pin.h"
#include "esp_system_native_port.h"
#include "flint_throw_support.h"

static FlintError checkPin(FlintExecution &execution, FlintInt8Array *pinsObj, uint32_t arrayLength) {
    uint8_t *pins = (uint8_t *)pinsObj->getData();
    for(uint8_t i = 0; i < arrayLength; i++) {
        const char *msg = NativePin_CheckPin(pins[i]);
        if(msg)
            return throwIOException(execution, msg);
    }
    return ERR_OK;
}

static FlintError checkParams(FlintExecution &execution, FlintInt8Array *pinsObj, uint32_t arrayLength) {
    if((pinsObj == NULL_PTR) || (arrayLength < 1) || (arrayLength > 32)) {
        if(pinsObj == NULL_PTR)
            return throwNullPointerException(execution, "pins array cannot be null object");
        else
            return throwIOException(execution, "The pin number must be from 1 to 32");
    }
    return checkPin(execution, pinsObj, arrayLength);
}

static FlintError nativeSetMode(FlintExecution &execution) {
    int32_t mode = execution.stackPopInt32();
    FlintInt8Array *pinsObj = (FlintInt8Array *)execution.stackPopObject();
    uint32_t arrayLength = pinsObj ? pinsObj->getLength() : 0;

    RETURN_IF_ERR(checkParams(execution, pinsObj, arrayLength));

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
        return throwIOException(execution, "Error while configuring the pin");
    return ERR_OK;
}

static FlintError nativeRead(FlintExecution &execution) {
    FlintJavaObject *obj = execution.stackPopObject();
    FlintInt8Array *pinsObj = (FlintInt8Array *)obj->getFields().getFieldObjectByIndex(0)->object;
    if(!pinsObj) {
        execution.stackPushInt32(0);
        return ERR_OK;
    }
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
    execution.stackPushInt32(value);
    return ERR_OK;
}

static FlintError nativeWrite(FlintExecution &execution) {
    uint32_t value = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    FlintInt8Array *pinsObj = (FlintInt8Array *)obj->getFields().getFieldObjectByIndex(0)->object;
    if(!pinsObj)
        return ERR_OK;
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
    return ERR_OK;
}

static FlintError nativeReset(FlintExecution &execution) {
    FlintJavaObject *obj = execution.stackPopObject();
    FlintInt8Array *pinsObj = (FlintInt8Array *)obj->getFields().getFieldObjectByIndex(0)->object;
    if(!pinsObj)
        return ERR_OK;
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
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x07\x00\xD2\x5C""setMode", "\x06\x00\xC6\x01""([BI)V", nativeSetMode),
    NATIVE_METHOD("\x04\x00\xDC\xC7""read",    "\x03\x00\xD0\x51""()I",    nativeRead),
    NATIVE_METHOD("\x05\x00\x03\xBB""write",   "\x04\x00\xB8\x03""(I)V",   nativeWrite),
    NATIVE_METHOD("\x05\x00\x27\x94""reset",   "\x03\x00\x91\x99""()V",    nativeReset),
};

const FlintNativeClass PORT_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x10\x00\xB8\x7E""esp/machine/Port", methods);
