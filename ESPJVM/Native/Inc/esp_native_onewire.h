

#ifndef __ESP_NATIVE_ONEWIRE_H
#define __ESP_NATIVE_ONEWIRE_H

#include "flint.h"
#include "flint_native.h"

jobject NativeOneWire_Open(FNIEnv *env, jobject obj);
jbool NativeOneWire_IsOpen(FNIEnv *env, jobject obj);
jvoid NativeOneWire_Reset(FNIEnv *env, jobject obj);
jint NativeOneWire_ReadByte(FNIEnv *env, jobject obj);
jint NativeOneWire_Read(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid NativeOneWire_WriteByte(FNIEnv *env, jobject obj, jint b);
jvoid NativeOneWire_Write(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid NativeOneWire_Close(FNIEnv *env, jobject obj);

inline constexpr NativeMethod oneWireMethods[] = {
    NATIVE_METHOD("open",   "()Lflint/machine/OneWire;", NativeOneWire_Open),
    NATIVE_METHOD("isOpen", "()Z",                       NativeOneWire_IsOpen),
    NATIVE_METHOD("reset",  "()V",                       NativeOneWire_Reset),
    NATIVE_METHOD("read",   "()I",                       NativeOneWire_ReadByte),
    NATIVE_METHOD("read",   "([BII)I",                   NativeOneWire_Read),
    NATIVE_METHOD("write",  "(I)V",                      NativeOneWire_WriteByte),
    NATIVE_METHOD("write",  "([BII)V",                   NativeOneWire_Write),
    NATIVE_METHOD("close",  "()V",                       NativeOneWire_Close),
};

#endif /* __ESP_NATIVE_ONEWIRE_H */
