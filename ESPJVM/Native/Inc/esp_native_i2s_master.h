

#ifndef __ESP_NATIVE_I2S_MASTER_H
#define __ESP_NATIVE_I2S_MASTER_H

#include "flint.h"
#include "flint_native.h"

void NativeI2sMaster_Reset(void);

jobject nativeI2sMasterOpen(FNIEnv *env, jobject obj);
jbool nativeI2sMasterIsOpen(FNIEnv *env, jobject obj);
jint nativeI2sMasterReadByte(FNIEnv *env, jobject obj);
jint nativeI2sMasterRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid nativeI2sMasterWriteByte(FNIEnv *env, jobject obj, jint b);
jvoid nativeI2sMasterWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid nativeI2sMasterClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod i2sMasterMethods[] = {
    NATIVE_METHOD("open",   "()Lflint/machine/I2sMaster;", nativeI2sMasterOpen),
    NATIVE_METHOD("isOpen", "()Z",                         nativeI2sMasterIsOpen),
    NATIVE_METHOD("read",   "()I",                         nativeI2sMasterReadByte),
    NATIVE_METHOD("read",   "([BII)I",                     nativeI2sMasterRead),
    NATIVE_METHOD("write",  "(I)V",                        nativeI2sMasterWriteByte),
    NATIVE_METHOD("write",  "([BII)V",                     nativeI2sMasterWrite),
    NATIVE_METHOD("close",  "()V",                         nativeI2sMasterClose),
};

#endif /* __ESP_NATIVE_I2S_MASTER_H */
