

#ifndef __ESP_NATIVE_WIFI_H
#define __ESP_NATIVE_WIFI_H

#include "flint_native.h"

jbool NativeWiFi_IsSupported(FNIEnv *env);
jvoid NativeWiFi_Connect(FNIEnv *env, jstring ssid, jstring password, jint authMode);
jbool NativeWiFi_IsConnected(FNIEnv *env);
jobject NativeWiFi_GetAPinfo(FNIEnv *env);
jvoid NativeWiFi_Disconnect(FNIEnv *env);
jvoid NativeWiFi_SoftAP(FNIEnv *env, jstring ssid, jstring password, jint authMode, jint channel, jint maxConnection);
jvoid NativeWiFi_SoftAPdisconnect(FNIEnv *env);
jvoid NativeWiFi_StartScan(FNIEnv *env, jbool blocked);
jobjectArray NativeWiFi_GetScanResult(FNIEnv *env);
jvoid NativeWiFi_StopScan(FNIEnv *env);

inline constexpr NativeMethod wifiMethods[] = {
    NATIVE_METHOD("isSupported",      "()Z",                                        NativeWiFi_IsSupported),

    NATIVE_METHOD("connect",          "(Ljava/lang/String;Ljava/lang/String;I)V",   NativeWiFi_Connect),
    NATIVE_METHOD("isConnected",      "()Z",                                        NativeWiFi_IsConnected),
    NATIVE_METHOD("getAPinfo",        "()Lflint/net/AccessPointRecord;",            NativeWiFi_GetAPinfo),
    NATIVE_METHOD("disconnect",       "()V",                                        NativeWiFi_Disconnect),

    NATIVE_METHOD("softAP",           "(Ljava/lang/String;Ljava/lang/String;III)V", NativeWiFi_SoftAP),
    NATIVE_METHOD("softAPdisconnect", "()V",                                        NativeWiFi_SoftAPdisconnect),

    NATIVE_METHOD("startScan",        "(Z)V",                                       NativeWiFi_StartScan),
    NATIVE_METHOD("getScanResults",   "()[Lflint/net/AccessPointRecord;",           NativeWiFi_GetScanResult),
    NATIVE_METHOD("stopScan",         "()V",                                        NativeWiFi_StopScan),
};

#endif /* __ESP_NATIVE_WIFI_H */
