

#ifndef __ESP_NATIVE_I2C_MASTER_H
#define __ESP_NATIVE_I2C_MASTER_H

#include "flint.h"
#include "flint_native.h"

void NativeI2cMaster_Reset(void);

jobject NativeI2cMaster_Open(FNIEnv *env, jobject obj);
jbool NativeI2cMaster_IsOpen(FNIEnv *env, jobject obj);
jint NativeI2cMaster_GetSpeed(FNIEnv *env, jobject obj);
jint NativeI2cMaster_ReadByte(FNIEnv *env, jobject obj);
jint NativeI2cMaster_Read(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid NativeI2cMaster_WriteByte(FNIEnv *env, jobject obj, jint b);
jvoid NativeI2cMaster_Write(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jint NativeI2cMaster_ReadMemByte(FNIEnv *env, jobject obj, jint memAddr);
jint NativeI2cMaster_ReadMem(FNIEnv *env, jobject obj, jint memAddr, jbyteArray b, jint off, jint count);
jvoid NativeI2cMaster_WriteMemByte(FNIEnv *env, jobject obj, jint memAddr, jint b);
jvoid NativeI2cMaster_WriteMem(FNIEnv *env, jobject obj, jint memAddr, jbyteArray buff, jint off, jint count);
jvoid NativeI2cMaster_Close(FNIEnv *env, jobject obj);

inline constexpr NativeMethod i2cMasterMethods[] = {
    NATIVE_METHOD("open",     "()Lflint/machine/I2cMaster;", NativeI2cMaster_Open),
    NATIVE_METHOD("isOpen",   "()Z",                         NativeI2cMaster_IsOpen),
    NATIVE_METHOD("getSpeed", "()I",                         NativeI2cMaster_GetSpeed),
    NATIVE_METHOD("read",     "()I",                         NativeI2cMaster_ReadByte),
    NATIVE_METHOD("read",     "([BII)I",                     NativeI2cMaster_Read),
    NATIVE_METHOD("write",    "(I)V",                        NativeI2cMaster_WriteByte),
    NATIVE_METHOD("write",    "([BII)V",                     NativeI2cMaster_Write),
    NATIVE_METHOD("readMem",  "(I)I",                        NativeI2cMaster_ReadMemByte),
    NATIVE_METHOD("readMem",  "(I[BII)I",                    NativeI2cMaster_ReadMem),
    NATIVE_METHOD("writeMem", "(II)V",                       NativeI2cMaster_WriteMemByte),
    NATIVE_METHOD("writeMem", "(I[BII)V",                    NativeI2cMaster_WriteMem),
    NATIVE_METHOD("close",    "()V",                         NativeI2cMaster_Close),
};

#endif /* __ESP_NATIVE_I2C_MASTER_H */
