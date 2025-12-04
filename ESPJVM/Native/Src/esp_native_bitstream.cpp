
#include "flint.h"
#include "rom/gpio.h"
#include "soc/gpio_reg.h"
#include "esp_rom_sys.h"
#include "flint_system_api.h"
#include "esp_native_common.h"
#include "esp_native_pin.h"
#include "esp_native_bitstream.h"

typedef class : public JObject {
public:
    jstring getName() { return (jstring)getFieldByIndex(0)->getObj(); }
    jint getId() { return getFieldByIndex(1)->getInt32(); }
    jint getEncoding() { return getFieldByIndex(2)->getInt32(); }
    jint getPin() { return getFieldByIndex(3)->getInt32(); }
    int32_t *getTiming() { return ((jintArray)getFieldByIndex(4)->getObj())->getData(); }

    void setId(int32_t val) { return getFieldByIndex(1)->setInt32(val); }
    void setPin(int32_t val) { return getFieldByIndex(3)->setInt32(val); }
} *BitStreamObject;

static bool NativeBitStream_IsOpen(int32_t id) {
    return (id >= 0) ? true : false;
}

static void NativeBitStreamWrite(BitStreamObject bitStream, uint8_t *buf, uint32_t count) {
    int32_t pin = bitStream->getPin();
    uint32_t mask, regSet, regClr, timing[4];
    uint32_t fcpu = esp_rom_get_cpu_ticks_per_us();
    int32_t encoding = bitStream->getEncoding();
#ifdef GPIO_OUT1_W1TC_REG
    if(pin < 32) {
        mask = 1 << pin;
        regSet = GPIO_OUT_W1TS_REG;
        regClr = GPIO_OUT_W1TC_REG;
    }
    else {
        mask = 1 << (pin - 32);
        regSet = GPIO_OUT1_W1TS_REG;
        regClr = GPIO_OUT1_W1TC_REG;
    }
#else
    mask = 1 << pin;
    regSet = GPIO_OUT_W1TS_REG;
    regClr = GPIO_OUT_W1TC_REG;
#endif
    for(int8_t i = 0; i < 4; i++) {
        timing[i] = fcpu * bitStream->getTiming()[i] / 1000;
        timing[i] = (timing[i] > 6) ? (timing[i] - 6) : 0;
        if(i % 2 == 1) timing[i] += timing[i - 1];
    }
    switch(encoding) {
        case 0:
            EnterAtomicSection();
            for(uint32_t i = 0; i < count; i++) {
                uint8_t b = buf[i];
                for(uint8_t j = 0; j < 8; ++j) {
                    GPIO_REG_WRITE(regSet, mask);
                    uint32_t start = GetCpuTicks();
                    uint32_t *t = &timing[b >> 6 & 2];
                    while(GetCpuTicks() - start < t[0]);
                    GPIO_REG_WRITE(regClr, mask);
                    b <<= 1;
                    while(GetCpuTicks() - start < t[1]);
                }
            }
            ExitAtomicSection();
            break;
        case 1:
            EnterAtomicSection();
            for(uint32_t i = 0; i < count; i++) {
                uint8_t b = buf[i];
                for(uint8_t j = 0; j < 8; ++j) {
                    GPIO_REG_WRITE(regClr, mask);
                    uint32_t start = GetCpuTicks();
                    uint32_t *t = &timing[b >> 6 & 2];
                    while(GetCpuTicks() - start < t[0]);
                    GPIO_REG_WRITE(regSet, mask);
                    b <<= 1;
                    while(GetCpuTicks() - start < t[1]);
                }
            }
            ExitAtomicSection();
            break;
        default:
            break;
    }
}

static bool CheckPrecondition(FNIEnv *env, BitStreamObject bitStream) {
    int32_t id = bitStream->getId();
    if(!NativeBitStream_IsOpen(id)) {
        env->throwNew(env->findClass("java/io/IOException"), "BitStream has not been opened yet");
        return false;
    }
    return true;
}

jobject NativeBitStream_Open(FNIEnv *env, jobject obj) {
    BitStreamObject bitStream = (BitStreamObject)obj;

    if(NativeBitStream_IsOpen(bitStream->getId())) {
        env->throwNew(env->findClass("java/io/IOException"), "BitStream is already open");
        return obj;
    }
    int32_t pin = bitStream->getPin();
    if(pin < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Output pin not specified");
        return obj;
    }
    if(const char *msg = NativePin_CheckPin(pin); msg != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return obj;
    }
    if(!NativePin_SetPinMode(1 << pin, 1)) {
        env->throwNew(env->findClass("java/io/IOException"), "Error while opening");
        return obj;
    }
    bitStream->setId(1);
    return obj;
}

jbool NativeBitStream_IsOpen(FNIEnv *env, jobject obj) {
    BitStreamObject bitStream = (BitStreamObject)obj;
    return NativeBitStream_IsOpen(bitStream->getId());
}

jvoid NativeBitStream_WriteByte(FNIEnv *env, jobject obj, jint b) {
    BitStreamObject bitStream = (BitStreamObject)obj;
    uint8_t buff = (uint8_t)b;
    if(!CheckPrecondition(env, bitStream)) return;
    NativeBitStreamWrite(bitStream, &buff, 1);
}

jvoid NativeBitStream_Write(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {
    BitStreamObject bitStream = (BitStreamObject)obj;
    uint8_t *buff = (uint8_t *)&b->getData()[off];
    if(!CheckPrecondition(env, bitStream)) return;
    if(!CheckArrayIndexSize(env, b, off, count)) return;
    NativeBitStreamWrite(bitStream, buff, count);
}

jvoid NativeBitStream_Close(FNIEnv *env, jobject obj) {
    BitStreamObject bitStream = (BitStreamObject)obj;
    bitStream->setId(-1);
}
