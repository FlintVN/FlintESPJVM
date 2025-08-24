

#ifndef __ESP_SYSTEM_NATIVE_PORT_H
#define __ESP_SYSTEM_NATIVE_PORT_H

#include "flint_native.h"

jvoid nativePortSetMode(FNIEnv *env, jbyteArray pinsObj, jint mode);
jint nativePortRead(FNIEnv *env, jobject obj);
jvoid nativePortWrite(FNIEnv *env, jobject obj, jint value);
jvoid nativePortReset(FNIEnv *env, jobject obj);

static constexpr NativeMethod portMethods[] = {
    NATIVE_METHOD("setMode", "([BI)V", nativePortSetMode),
    NATIVE_METHOD("read",    "()I",    nativePortRead),
    NATIVE_METHOD("write",   "(I)V",   nativePortWrite),
    NATIVE_METHOD("reset",   "()V",    nativePortReset),
};

#endif /* __ESP_SYSTEM_NATIVE_PORT_H */
