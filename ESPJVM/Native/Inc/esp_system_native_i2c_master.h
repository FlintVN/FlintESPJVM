

#ifndef __ESP_SYSTEM_NATIVE_I2C_MASTER_H
#define __ESP_SYSTEM_NATIVE_I2C_MASTER_H

#include "flint.h"
#include "flint_native.h"

void NativeI2cMaster_Reset(void);

jobject nativeI2cMasterOpen(FNIEnv *env, jobject obj);
jbool nativeI2cMasterIsOpen(FNIEnv *env, jobject obj);
jint nativeI2cMasterGetSpeed(FNIEnv *env, jobject obj);
jint nativeI2cMasterReadByte(FNIEnv *env, jobject obj);
jint nativeI2cMasterRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid nativeI2cMasterWriteByte(FNIEnv *env, jobject obj, jint b);
jvoid nativeI2cMasterWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jint nativeI2cMasterReadMemByte(FNIEnv *env, jobject obj, jint memAddr);
jint nativeI2cMasterReadMem(FNIEnv *env, jobject obj, jint memAddr, jbyteArray b, jint off, jint count);
jvoid nativeI2cMasterWriteMemByte(FNIEnv *env, jobject obj, jint memAddr, jint b);
jvoid nativeI2cMasterWriteMem(FNIEnv *env, jobject obj, jint memAddr, jbyteArray buff, jint off, jint count);
jvoid nativeI2cMasterClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod i2cMasterMethods[] = {
    NATIVE_METHOD("open",     "()Lflint/machine/I2cMaster;", nativeI2cMasterOpen),
    NATIVE_METHOD("isOpen",   "()Z",                         nativeI2cMasterIsOpen),
    NATIVE_METHOD("getSpeed", "()I",                         nativeI2cMasterGetSpeed),
    NATIVE_METHOD("read",     "()I",                         nativeI2cMasterReadByte),
    NATIVE_METHOD("read",     "([BII)I",                     nativeI2cMasterRead),
    NATIVE_METHOD("write",    "(I)V",                        nativeI2cMasterWriteByte),
    NATIVE_METHOD("write",    "([BII)V",                     nativeI2cMasterWrite),
    NATIVE_METHOD("readMem",  "(I)I",                        nativeI2cMasterReadMemByte),
    NATIVE_METHOD("readMem",  "(I[BII)I",                    nativeI2cMasterReadMem),
    NATIVE_METHOD("writeMem", "(II)V",                       nativeI2cMasterWriteMemByte),
    NATIVE_METHOD("writeMem", "(I[BII)V",                    nativeI2cMasterWriteMem),
    NATIVE_METHOD("close",    "()V",                         nativeI2cMasterClose),
};

#endif /* __ESP_SYSTEM_NATIVE_I2C_MASTER_H */
