#ifndef __ESP_NATIVE_CAN_H
#define __ESP_NATIVE_CAN_H

#include "flint_native.h"

#ifdef __cplusplus
extern "C" {
#endif

// Global reset function
void NativeCan_Reset(void);

// Core CAN operations
jobject  NativeCan_Open(FNIEnv* env, jobject obj);
jvoid    NativeCan_Close(FNIEnv* env, jobject obj);
jbool    NativeCan_IsOpen(FNIEnv* env, jobject obj);
jvoid    NativeCan_Start(FNIEnv* env, jobject obj);
jvoid    NativeCan_Stop(FNIEnv* env, jobject obj);
jvoid    NativeCan_Recover(FNIEnv* env, jobject obj);

// Alert configuration
jvoid    NativeCan_ConfigureAlerts(FNIEnv* env, jobject obj, jlong alertsMask);
jlong    NativeCan_ReadAlerts(FNIEnv* env, jobject obj, jlong timeoutMs);

// Status monitoring
jobject  NativeCan_GetStatusInfo(FNIEnv* env, jobject obj);
jvoid    NativeCan_ClearTxQueue(FNIEnv* env, jobject obj);
jvoid    NativeCan_ClearRxQueue(FNIEnv* env, jobject obj);

// Data transfer
jvoid    NativeCan_Write(FNIEnv* env, jobject obj, jbyteArray data);
jbyteArray NativeCan_ReadFrame(FNIEnv* env, jobject obj, jlong timeoutMs);
jobject  NativeCan_ReadMessage(FNIEnv* env, jobject obj, jlong timeoutMs);
jvoid    NativeCan_WriteMessage(FNIEnv* env, jobject obj, jobject msg, jlong timeoutMs);
jobject  NativeCan_ReceiveMessage(FNIEnv* env, jobject obj, jlong timeoutMs);

#ifdef __cplusplus
}
#endif

// Method table for registration with JVM
inline constexpr NativeMethod canMethods[] = {
    NATIVE_METHOD("open",               "()Lflint/machine/Can;",                NativeCan_Open),
    NATIVE_METHOD("close",              "()V",                                  NativeCan_Close),
    NATIVE_METHOD("isOpen",             "()Z",                                  NativeCan_IsOpen),
    NATIVE_METHOD("start",              "()V",                                  NativeCan_Start),
    NATIVE_METHOD("stop",               "()V",                                  NativeCan_Stop),
    NATIVE_METHOD("recover",            "()V",                                  NativeCan_Recover),
    NATIVE_METHOD("configureAlerts",    "(J)V",                                 NativeCan_ConfigureAlerts),    
    NATIVE_METHOD("readAlerts",         "(J)J",                                 NativeCan_ReadAlerts),
    NATIVE_METHOD("getStatus",          "()Lflint/machine/CanStatus;",          NativeCan_GetStatusInfo),
    NATIVE_METHOD("clearTxQueue",       "()V",                                  NativeCan_ClearTxQueue),
    NATIVE_METHOD("clearRxQueue",       "()V",                                  NativeCan_ClearRxQueue),
    NATIVE_METHOD("write",              "([B)V",                                NativeCan_Write),
    NATIVE_METHOD("readFrame",          "(J)[B",                                NativeCan_ReadFrame),
    NATIVE_METHOD("readMessage",        "(J)Lflint/machine/CanMessage;",        NativeCan_ReadMessage),
    NATIVE_METHOD("writeMessage",       "(Lflint/machine/CanMessage;J)V",       NativeCan_WriteMessage),
    NATIVE_METHOD("receiveMessage",     "(J)Lflint/machine/CanMessage;",        NativeCan_ReceiveMessage)
};

#endif /* __ESP_NATIVE_CAN_H */