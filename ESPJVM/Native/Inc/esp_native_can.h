

#ifndef __ESP_NATIVE_CAN_H
#define __ESP_NATIVE_CAN_H

#include "flint_native.h"

void NativeCan_Reset(void);

jvoid NativeCan_InitCan(FNIEnv *env, jobject obj);
jvoid NativeCan_DeinitCan(FNIEnv *env, jobject obj);
jint NativeCan_IsOpen(FNIEnv *env, jobject obj);
jvoid NativeCan_Open(FNIEnv *env, jobject obj);
jvoid NativeCan_Close(FNIEnv *env, jobject obj);
jvoid NativeCan_Start(FNIEnv *env, jobject obj);
jvoid NativeCan_Stop(FNIEnv *env, jobject obj);
jvoid NativeCan_Recover(FNIEnv *env, jobject obj);
jvoid NativeCan_Write(FNIEnv *env, jobject obj, jint data);
jvoid NativeCan_WriteMessage(FNIEnv *env, jobject obj, jobject msgObj);
jvoid NativeCan_ReadFrame(FNIEnv *env, jobject obj);
jvoid NativeCan_ReadMessage(FNIEnv *env, jobject obj);
jvoid NativeCan_ConfigureAlerts(FNIEnv *env, jobject obj, jlong mask);
jvoid NativeCan_ReadAlerts(FNIEnv *env, jobject obj, jlong timeout);
jvoid NativeCan_GetStatusInfo(FNIEnv *env, jobject obj);
jvoid NativeCan_ClearTxQueue(FNIEnv *env, jobject obj);
jvoid NativeCan_ClearRxQueue(FNIEnv *env, jobject obj);


inline constexpr NativeMethod canMethods[] = {
    NATIVE_METHOD("initCan", "()V",                             NativeCan_InitCan),
    NATIVE_METHOD("deinitCan", "()V",                           NativeCan_DeinitCan),
    NATIVE_METHOD("isOpen", "()Z",                              NativeCan_IsOpen),
    NATIVE_METHOD("open", "()V",                                NativeCan_Open),
    NATIVE_METHOD("close", "()V",                               NativeCan_Close),
    NATIVE_METHOD("start", "()V",                               NativeCan_Start),
    NATIVE_METHOD("stop", "()V",                                NativeCan_Stop),
    NATIVE_METHOD("recover", "()V",                             NativeCan_Recover),
    NATIVE_METHOD("write", "(I)V",                              NativeCan_Write),
    NATIVE_METHOD("writeMessage", "(Lcom/example/Message;)V",   NativeCan_WriteMessage),
    NATIVE_METHOD("readFrame", "()I",                           NativeCan_ReadFrame),
    NATIVE_METHOD("readMessage", "()Lcom/example/Message;",     NativeCan_ReadMessage),
    NATIVE_METHOD("configureAlerts", "(J)V",                    NativeCan_ConfigureAlerts),
    NATIVE_METHOD("readAlerts", "(J)V",                         NativeCan_ReadAlerts),
    NATIVE_METHOD("getStatusInfo", "()V",                       NativeCan_GetStatusInfo),
    NATIVE_METHOD("clearTxQueue", "()V",                        NativeCan_ClearTxQueue),
    NATIVE_METHOD("clearRxQueue", "()V" ,                       NativeCan_ClearRxQueue)
};

#endif /* __ESP_NATIVE_CAN_H */
