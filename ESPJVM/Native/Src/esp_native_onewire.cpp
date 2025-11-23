
#include "flint.h"
#include "rom/gpio.h"
#include "soc/gpio_reg.h"
#include "esp_rom_sys.h"
#include "flint_system_api.h"
#include "esp_native_common.h"
#include "esp_native_pin.h"
#include "esp_native_onewire.h"

typedef class : public JObject {
public:
    jstring getName() { return (jstring)getFieldByIndex(0)->getObj(); }
    jint getId() { return getFieldByIndex(1)->getInt32(); }
    jint getSpeedMode() { return getFieldByIndex(2)->getInt32(); }
    jint getPin() { return getFieldByIndex(3)->getInt32(); }

    void setId(int32_t val) { return getFieldByIndex(1)->setInt32(val); }
    void setSpeedMode(int32_t val) { return getFieldByIndex(2)->setInt32(val); }
    void setPin(int32_t val) { return getFieldByIndex(3)->setInt32(val); }
} *OneWireObject;

static bool NativeOneWire_IsOpen(int32_t id) {
    return (id >= 0) ? true : false;
}

static void GetPinParams(uint32_t pin, uint32_t *mask, uint32_t *set, uint32_t *clr, uint32_t *in) {
#ifdef GPIO_OUT1_W1TC_REG
    if(pin < 32) {
        *mask = 1 << pin;
        *set = GPIO_OUT_W1TS_REG;
        *clr = GPIO_OUT_W1TC_REG;
        *in = GPIO_IN_REG;
    }
    else {
        *mask = 1 << (pin - 32);
        *set = GPIO_OUT1_W1TS_REG;
        *clr = GPIO_OUT1_W1TC_REG;
        *in = GPIO_IN1_REG;
    }
#else
    *mask = 1 << pin;
    *set = GPIO_OUT_W1TS_REG;
    *clr = GPIO_OUT_W1TC_REG;
    *in = GPIO_IN_REG;
#endif
}

static void NativeOneWireWrite(int32_t pin, int32_t speedMode, uint8_t *buff, uint32_t count) {
    uint32_t mask, set, clr, in;
    uint32_t fcpu = esp_rom_get_cpu_ticks_per_us();
    uint32_t timing[3];
    if(speedMode == 0) {
        timing[0] = 6 * fcpu - 25;
        timing[1] = 60 * fcpu - 25;
        timing[2] = 70 * fcpu - 25;
    }
    else {
        timing[0] = 2 * fcpu - 25;
        timing[1] = 6 * fcpu - 25;
        timing[2] = 8 * fcpu - 25;
    }
    GetPinParams(pin, &mask, &set, &clr, &in);

    for(uint32_t i = 0; i < count; i++) {
        uint8_t b = buff[i];
        for(uint8_t j = 0; j < 8; j++) {
            EnterAtomicSection();
            REG_WRITE(clr, mask);
            uint32_t start = GetCpuTicks();
            uint32_t bit = (b & 0x01) ? set : clr;
            while(GetCpuTicks() - start < timing[0]);
            REG_WRITE(bit, mask);
            b >>= 1;
            while(GetCpuTicks() - start < timing[1]);
            REG_WRITE(set, mask);
            while(GetCpuTicks() - start < timing[2]);
            ExitAtomicSection();
        }
    }
}

static void NativeOneWireRead(int32_t pin, int32_t speedMode, uint8_t *buff, uint32_t count) {
    uint32_t mask, set, clr, in;
    uint32_t fcpu = esp_rom_get_cpu_ticks_per_us();
    uint32_t timing[4];
    if(speedMode == 0) {
        timing[0] = 6 * fcpu - 25;
        timing[1] = 15 * fcpu - 25;
        timing[2] = 60 * fcpu - 25;
        timing[3] = 70 * fcpu - 25;
    }
    else {
        timing[0] = 2 * fcpu - 25;
        timing[1] = 3 * fcpu - 25;
        timing[2] = 6 * fcpu - 25;
        timing[3] = 8 * fcpu - 25;
    }
    GetPinParams(pin, &mask, &set, &clr, &in);

    for(uint32_t i = 0; i < count; i++) {
        uint8_t b = 0;
        for(uint8_t j = 0; j < 8; j++) {
            EnterAtomicSection();
            REG_WRITE(clr, mask);
            uint32_t start = GetCpuTicks();
            while(GetCpuTicks() - start < timing[0]);
            REG_WRITE(set, mask);
            b >>= 1;
            while(GetCpuTicks() - start < timing[1]);
            b |= (REG_READ(in) & mask) ? 0x80 : 0x00;
            while(GetCpuTicks() - start < timing[2]);
            REG_WRITE(set, mask);
            while(GetCpuTicks() - start < timing[3]);
            ExitAtomicSection();
        }
        buff[i] = b;
    }
}

static bool NativeOneWireReset(int32_t pin, int32_t speedMode) {
    uint8_t ret = 0;
    uint32_t mask, set, clr, in;
    uint32_t fcpu = esp_rom_get_cpu_ticks_per_us();
    uint32_t resetLow = ((speedMode == 2) ? 48 : 480) * fcpu - 25;
    uint32_t resetTime = ((speedMode == 2) ? 80 : 960) * fcpu - 25;
    GetPinParams(pin, &mask, &set, &clr, &in);

    REG_WRITE(clr, mask);
    uint32_t start = GetCpuTicks();
    while(GetCpuTicks() - start < resetLow);
    EnterAtomicSection();
    REG_WRITE(set, mask);
    while((GetCpuTicks() - start < resetTime) && !(REG_READ(in) & mask));
    while(GetCpuTicks() - start < resetTime) {
        if(!(REG_READ(in) & mask)) {
            ret++;
            break;
        }
    }
    while(GetCpuTicks() - start < resetTime) {
        if(REG_READ(in) & mask) {
            ret++;
            break;
        }
    }
    ExitAtomicSection();

    if(ret == 2) {
        start = GetCpuTicks();
        while(GetCpuTicks() - start < (40 * fcpu - 25));
    }

    if(ret == 2 && speedMode == 1) {
        uint8_t b = 0x3C;   /* OVERDRIVE SKIP ROM */
        NativeOneWireWrite(pin, 0, &b, 1);
    }

    return ret == 2;
}

static bool checkPrecondition(FNIEnv *env, OneWireObject oneWire) {
    int32_t id = oneWire->getId();
    if(!NativeOneWire_IsOpen(id)) {
        env->throwNew(env->findClass("java/io/IOException"), "BitStream has not been opened yet");
        return false;
    }
    return true;
}

jobject nativeOneWireOpen(FNIEnv *env, jobject obj) {
    OneWireObject oneWire = (OneWireObject)obj;

    if(NativeOneWire_IsOpen(oneWire->getId())) {
        env->throwNew(env->findClass("java/io/IOException"), "OneWire is already open");
        return obj;
    }
    int32_t pin = oneWire->getPin();
    if(pin < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Output pin not specified");
        return obj;
    }
    if(const char *msg = NativePin_CheckPin(pin); msg != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return obj;
    }

    oneWire->setId(1);
    if(!NativePin_SetPinMode(1 << pin, 5)) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while opening");
        return obj;
    }
#ifdef GPIO_OUT1_W1TS_REG
    if(pin < 32)
        REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);
    else
        REG_WRITE(GPIO_OUT1_W1TS_REG, 1 << (pin - 32));
#else
    REG_WRITE(GPIO_OUT_W1TS_REG, 1 << pin);
#endif
    return obj;
}

jbool nativeOneWireIsOpen(FNIEnv *env, jobject obj) {
    OneWireObject oneWire = (OneWireObject)obj;
    return NativeOneWire_IsOpen(oneWire->getId());
}

jvoid nativeOneWireReset(FNIEnv *env, jobject obj) {
    OneWireObject oneWire = (OneWireObject)obj;
    if(!checkPrecondition(env, oneWire)) return;
    if(!NativeOneWireReset(oneWire->getPin(), oneWire->getSpeedMode()))
        env->throwNew(env->findClass("java/io/IOException"), "No devices present");
}

jint nativeOneWireReadByte(FNIEnv *env, jobject obj) {
    OneWireObject oneWire = (OneWireObject)obj;
    uint8_t buff;
    if(!checkPrecondition(env, oneWire)) return -1;
    NativeOneWireRead(oneWire->getPin(), oneWire->getSpeedMode(), &buff, 1);
    return buff;
}

jint nativeOneWireRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {
    OneWireObject oneWire = (OneWireObject)obj;
    uint8_t *buff = (uint8_t *)&b->getData()[off];
    if(!checkPrecondition(env, oneWire)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;
    NativeOneWireRead(oneWire->getPin(), oneWire->getSpeedMode(), buff, count);
    return count;
}

jvoid nativeOneWireWriteByte(FNIEnv *env, jobject obj, jint b) {
    OneWireObject oneWire = (OneWireObject)obj;
    uint8_t buff = (uint8_t)b;
    if(!checkPrecondition(env, oneWire)) return;
    NativeOneWireWrite(oneWire->getPin(), oneWire->getSpeedMode(), &buff, 1);
}

jvoid nativeOneWireWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {
    OneWireObject oneWire = (OneWireObject)obj;
    uint8_t *buff = (uint8_t *)&b->getData()[off];
    if(!checkPrecondition(env, oneWire)) return;
    if(!CheckArrayIndexSize(env, b, off, count)) return;
    NativeOneWireWrite(oneWire->getPin(), oneWire->getSpeedMode(), buff, count);
}

jvoid nativeOneWireClose(FNIEnv *env, jobject obj) {
    OneWireObject oneWire = (OneWireObject)obj;
    oneWire->setId(-1);
}
