

#ifndef __ESP_NATIVE_SPI_H
#define __ESP_NATIVE_SPI_H

#include "flint.h"
#include "flint_native.h"

void NativeSpiMaster_Reset(void);

jobject nativeSpiMasterOpen(FNIEnv *env, jobject obj);
jbool nativeSpiMasterIsOpen(FNIEnv *env, jobject obj);
jint nativeSpiMasterGetSpeed(FNIEnv *env, jobject obj);
jint nativeSpiMasterRead(FNIEnv *env, jobject obj);
jvoid nativeSpiMasterWrite(FNIEnv *env, jobject obj, jint b);
jint nativeSpiMasterReadWrite(FNIEnv *env, jobject obj, jbyteArray tx, jint txOff, jboolArray rx, jint rxOff, jint length);
jvoid nativeSpiMasterClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod spiMasterMethods[] = {
    NATIVE_METHOD("open",            "()Lflint/machine/SpiMaster;", nativeSpiMasterOpen),
    NATIVE_METHOD("isOpen",          "()Z",                         nativeSpiMasterIsOpen),
    NATIVE_METHOD("getSpeed",        "()I",                         nativeSpiMasterGetSpeed),
    NATIVE_METHOD("read",            "()I",                         nativeSpiMasterRead),
    NATIVE_METHOD("write",           "(I)V",                        nativeSpiMasterWrite),
    NATIVE_METHOD("readWrite",       "([BI[BII)I",                  nativeSpiMasterReadWrite),
    NATIVE_METHOD("close",           "()V",                         nativeSpiMasterClose),
};

#endif /* __ESP_NATIVE_SPI_H */
