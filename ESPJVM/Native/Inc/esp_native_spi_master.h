

#ifndef __ESP_NATIVE_SPI_H
#define __ESP_NATIVE_SPI_H

#include "flint.h"
#include "flint_native.h"

void NativeSpiMaster_Reset(void);

jobject NativeSpiMaster_Open(FNIEnv *env, jobject obj);
jbool NativeSpiMaster_IsOpen(FNIEnv *env, jobject obj);
jint NativeSpiMaster_GetSpeed(FNIEnv *env, jobject obj);
jint NativeSpiMaster_Read(FNIEnv *env, jobject obj);
jvoid NativeSpiMaster_Write(FNIEnv *env, jobject obj, jint b);
jint NativeSpiMaster_ReadWrite(FNIEnv *env, jobject obj, jbyteArray tx, jint txOff, jboolArray rx, jint rxOff, jint length);
jvoid NativeSpiMaster_Close(FNIEnv *env, jobject obj);

static constexpr NativeMethod spiMasterMethods[] = {
    NATIVE_METHOD("open",            "()Lflint/machine/SpiMaster;", NativeSpiMaster_Open),
    NATIVE_METHOD("isOpen",          "()Z",                         NativeSpiMaster_IsOpen),
    NATIVE_METHOD("getSpeed",        "()I",                         NativeSpiMaster_GetSpeed),
    NATIVE_METHOD("read",            "()I",                         NativeSpiMaster_Read),
    NATIVE_METHOD("write",           "(I)V",                        NativeSpiMaster_Write),
    NATIVE_METHOD("readWrite",       "([BI[BII)I",                  NativeSpiMaster_ReadWrite),
    NATIVE_METHOD("close",           "()V",                         NativeSpiMaster_Close),
};

#endif /* __ESP_NATIVE_SPI_H */
