

#ifndef __ESP_NATIVE_I2S_MASTER_H
#define __ESP_NATIVE_I2S_MASTER_H

#include "flint.h"
#include "flint_native.h"

void NativeI2sMaster_Reset(void);

jobject NativeI2sMaster_Open(FNIEnv *env, jobject obj);
jbool NativeI2sMaster_IsOpen(FNIEnv *env, jobject obj);

jint NativeI2sMaster_ReadByte(FNIEnv *env, jobject obj);
jint NativeI2sMaster_ReadByteArray(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jint NativeI2sMaster_ReadShortArray(FNIEnv *env, jobject obj, jshortArray b, jint off, jint count);
jint NativeI2sMaster_ReadIntArray(FNIEnv *env, jobject obj, jintArray b, jint off, jint count);

jvoid NativeI2sMaster_WriteByte(FNIEnv *env, jobject obj, jint b);
jvoid NativeI2sMaster_WriteByteArray(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count);
jvoid NativeI2sMaster_WriteShortArray(FNIEnv *env, jobject obj, jshortArray b, jint off, jint count);
jvoid NativeI2sMaster_WriteIntArray(FNIEnv *env, jobject obj, jintArray b, jint off, jint count);

jvoid NativeI2sMaster_Close(FNIEnv *env, jobject obj);

inline constexpr NativeMethod i2sMasterMethods[] = {
    NATIVE_METHOD("open",   "()Lflint/io/I2sMaster;", NativeI2sMaster_Open),
    NATIVE_METHOD("isOpen", "()Z",                    NativeI2sMaster_IsOpen),
    NATIVE_METHOD("read",   "()I",                    NativeI2sMaster_ReadByte),
    NATIVE_METHOD("read",   "([BII)I",                NativeI2sMaster_ReadByteArray),
    NATIVE_METHOD("write",  "(I)V",                   NativeI2sMaster_WriteByte),
    NATIVE_METHOD("write",  "([BII)V",                NativeI2sMaster_WriteByteArray),
    NATIVE_METHOD("close",  "()V",                    NativeI2sMaster_Close),
};

#endif /* __ESP_NATIVE_I2S_MASTER_H */
