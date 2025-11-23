

#ifndef __ESP_NATIVE_ONEWIRE_H
#define __ESP_NATIVE_ONEWIRE_H

#include "flint.h"
#include "flint_native.h"

jobject nativeOneWireOpen(FNIEnv *env, jobject obj);
jbool nativeOneWireIsOpen(FNIEnv *env, jobject obj);
jvoid nativeOneWireReset(FNIEnv *env, jobject obj);
jint nativeOneWireReadByte(FNIEnv *env, jobject obj);
jint nativeOneWireRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid nativeOneWireWriteByte(FNIEnv *env, jobject obj, jint b);
jvoid nativeOneWireWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid nativeOneWireClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod oneWireMethods[] = {
    NATIVE_METHOD("open",   "()Lflint/machine/OneWire;", nativeOneWireOpen),
    NATIVE_METHOD("isOpen", "()Z",                       nativeOneWireIsOpen),
    NATIVE_METHOD("reset",  "()V",                       nativeOneWireReset),
    NATIVE_METHOD("read",   "()I",                       nativeOneWireReadByte),
    NATIVE_METHOD("read",   "([BII)I",                   nativeOneWireRead),
    NATIVE_METHOD("write",  "(I)V",                      nativeOneWireWriteByte),
    NATIVE_METHOD("write",  "([BII)V",                   nativeOneWireWrite),
    NATIVE_METHOD("close",  "()V",                       nativeOneWireClose),
};

#endif /* __ESP_NATIVE_ONEWIRE_H */
