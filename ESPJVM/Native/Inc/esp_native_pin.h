
#ifndef __ESP_NATIVE_PIN_H
#define __ESP_NATIVE_PIN_H

#include "flint_native.h"

const char *NativePin_CheckPin(int32_t pn);
bool NativePin_SetPinMode(int32_t pinMask, uint32_t mode);
void NativePin_Reset(void);

jvoid NativePin_SetMode(FNIEnv *env, jint pin, jint mode);
jbool NativePin_Read(FNIEnv *env, jobject obj);
jvoid NativePin_Write(FNIEnv *env, jobject obj, jbool level);
jvoid NativePin_LogicSet(FNIEnv *env, jobject obj);
jvoid NativePin_LogicReset(FNIEnv *env, jobject obj);
jvoid NativePin_Toggle(FNIEnv *env, jobject obj);

inline constexpr NativeMethod pinMethods[] = {
    NATIVE_METHOD("setMode", "(II)V", NativePin_SetMode),
    NATIVE_METHOD("read",    "()Z",   NativePin_Read),
    NATIVE_METHOD("write",   "(Z)V",  NativePin_Write),
    NATIVE_METHOD("set",     "()V",   NativePin_LogicSet),
    NATIVE_METHOD("reset",   "()V",   NativePin_LogicReset),
    NATIVE_METHOD("toggle",  "()V",   NativePin_Toggle),
};

#endif /* __ESP_NATIVE_PIN_H */
