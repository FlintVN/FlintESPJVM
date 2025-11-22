

#ifndef __ESP_NATIVE_PIN_H
#define __ESP_NATIVE_PIN_H

#include "flint_native.h"

const char *NativePin_CheckPin(int32_t pin);
bool NativePin_SetPinMode(int32_t pinMask, uint32_t mode);
void NativePin_Reset(void);

jvoid nativePinSetMode(FNIEnv *env, jint pin, jint mode);
jbool nativePinRead(FNIEnv *env, jobject obj);
jvoid nativePinWrite(FNIEnv *env, jobject obj, jbool level);
jvoid nativePinSet(FNIEnv *env, jobject obj);
jvoid nativePinReset(FNIEnv *env, jobject obj);
jvoid nativePinToggle(FNIEnv *env, jobject obj);

static constexpr NativeMethod pinMethods[] = {
    NATIVE_METHOD("setMode", "(II)V", nativePinSetMode),
    NATIVE_METHOD("read",    "()Z",   nativePinRead),
    NATIVE_METHOD("write",   "(Z)V",  nativePinWrite),
    NATIVE_METHOD("set",     "()V",   nativePinSet),
    NATIVE_METHOD("reset",   "()V",   nativePinReset),
    NATIVE_METHOD("toggle",  "()V",   nativePinToggle),
};

#endif /* __ESP_NATIVE_PIN_H */
