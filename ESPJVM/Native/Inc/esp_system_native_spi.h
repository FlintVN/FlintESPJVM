

#ifndef __ESP_SYSTEM_NATIVE_SPI_H
#define __ESP_SYSTEM_NATIVE_SPI_H

#include "flint.h"
#include "flint_native.h"

void NativeSPI_Reset();

jobject nativeSpiOpen(FNIEnv *env, jobject obj);
jbool nativeSpiIsOpen(FNIEnv *env, jobject obj);
jint nativeSpiGetSpeed(FNIEnv *env, jobject obj);
jint nativeSpiRead(FNIEnv *env, jobject obj);
jvoid nativeSpiWrite(FNIEnv *env, jobject obj, jint b);
jint nativeSpiReadWrite(FNIEnv *env, jobject obj, jbyteArray tx, jint txOff, jboolArray rx, jint rxOff, jint length);
jvoid nativeSpiClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod spiMethods[] = {
    NATIVE_METHOD("open",            "()Lflint/machine/SpiMaster;", nativeSpiOpen),
    NATIVE_METHOD("isOpen",          "()Z",                         nativeSpiIsOpen),
    NATIVE_METHOD("getSpeed",        "()I",                         nativeSpiGetSpeed),
    NATIVE_METHOD("read",            "()I",                         nativeSpiRead),
    NATIVE_METHOD("write",           "(I)V",                        nativeSpiWrite),
    NATIVE_METHOD("readWrite",       "([BI[BII)I",                  nativeSpiReadWrite),
    NATIVE_METHOD("close",           "()V",                         nativeSpiClose),
};

#endif /* __ESP_SYSTEM_NATIVE_SPI_H */
