

#ifndef __ESP_NATIVE_PORT_H
#define __ESP_NATIVE_PORT_H

#include "flint_native.h"

jvoid NativePort_SetMode(FNIEnv *env, jbyteArray pinsObj, jint mode);
jint NativePort_Read(FNIEnv *env, jobject obj);
jvoid NativePort_Write(FNIEnv *env, jobject obj, jint value);
jvoid NativePort_Reset(FNIEnv *env, jobject obj);

static constexpr NativeMethod portMethods[] = {
    NATIVE_METHOD("setMode", "([BI)V", NativePort_SetMode),
    NATIVE_METHOD("read",    "()I",    NativePort_Read),
    NATIVE_METHOD("write",   "(I)V",   NativePort_Write),
    NATIVE_METHOD("reset",   "()V",    NativePort_Reset),
};

#endif /* __ESP_NATIVE_PORT_H */
