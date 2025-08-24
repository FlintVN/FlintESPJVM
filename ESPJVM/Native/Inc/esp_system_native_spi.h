

#ifndef __ESP_SYSTEM_NATIVE_SPI_H
#define __ESP_SYSTEM_NATIVE_SPI_H

#include "flint.h"
#include "flint_native.h"

void NativeSPI_Reset();

jvoid nativeSpiInitInstance(FNIEnv *env, jobject obj);
jvoid nativeSpiOpen(FNIEnv *env, jobject obj);
jbool nativeSpiIsOpen(FNIEnv *env, jobject obj);
jint nativeSpiGetSpeed(FNIEnv *env, jobject obj);
jint nativeSpiReadWrite(FNIEnv *env, jobject obj, jbyteArray txBuff, jint txOff, jboolArray rxBuff, jint rxOff, jint length);
jvoid nativeSpiClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod spiMethods[] = {
    NATIVE_METHOD("initInstance", "()V",        nativeSpiInitInstance),
    NATIVE_METHOD("open",         "()V",        nativeSpiOpen),
    NATIVE_METHOD("isOpen",       "()Z",        nativeSpiIsOpen),
    NATIVE_METHOD("getSpeed",     "()I",        nativeSpiGetSpeed),
    NATIVE_METHOD("readWrite",    "([BI[BII)I", nativeSpiReadWrite),
    NATIVE_METHOD("close",        "()V",        nativeSpiClose),
};

#endif /* __ESP_SYSTEM_NATIVE_SPI_H */
