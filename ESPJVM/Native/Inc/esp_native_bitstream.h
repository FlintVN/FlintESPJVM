

#ifndef __ESP_NATIVE_BITSTREAM_H
#define __ESP_NATIVE_BITSTREAM_H

#include "flint.h"
#include "flint_native.h"

jobject NativeBitStream_Open(FNIEnv *env, jobject obj);
jbool NativeBitStream_IsOpen(FNIEnv *env, jobject obj);
jvoid NativeBitStream_WriteByte(FNIEnv *env, jobject obj, jint b);
jvoid NativeBitStream_Write(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid NativeBitStream_Close(FNIEnv *env, jobject obj);

inline constexpr NativeMethod bitStreamMethods[] = {
    NATIVE_METHOD("open",   "()Lflint/machine/BitStream;", NativeBitStream_Open),
    NATIVE_METHOD("isOpen", "()Z",                         NativeBitStream_IsOpen),
    NATIVE_METHOD("write",  "(I)V",                        NativeBitStream_WriteByte),
    NATIVE_METHOD("write",  "([BII)V",                     NativeBitStream_Write),
    NATIVE_METHOD("close",  "()V",                         NativeBitStream_Close),
};

#endif /* __ESP_NATIVE_BITSTREAM_H */
