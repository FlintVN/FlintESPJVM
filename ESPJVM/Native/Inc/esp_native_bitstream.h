

#ifndef __ESP_NATIVE_BITSTREAM_H
#define __ESP_NATIVE_BITSTREAM_H

#include "flint.h"
#include "flint_native.h"

jobject nativeBitStreamOpen(FNIEnv *env, jobject obj);
jbool nativeBitStreamIsOpen(FNIEnv *env, jobject obj);
jvoid nativeBitStreamWriteByte(FNIEnv *env, jobject obj, jint b);
jvoid nativeBitStreamWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid nativeBitStreamClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod bitStreamMethods[] = {
    NATIVE_METHOD("open",   "()Lflint/machine/BitStream;", nativeBitStreamOpen),
    NATIVE_METHOD("isOpen", "()Z",                         nativeBitStreamIsOpen),
    NATIVE_METHOD("write",  "(I)V",                        nativeBitStreamWriteByte),
    NATIVE_METHOD("write",  "([BII)V",                     nativeBitStreamWrite),
    NATIVE_METHOD("close",  "()V",                         nativeBitStreamClose),
};

#endif /* __ESP_NATIVE_BITSTREAM_H */
