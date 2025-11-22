

#ifndef __ESP_NATIVE_WIFI_H
#define __ESP_NATIVE_WIFI_H

#include "flint_native.h"

jbool nativeWiFiIsSupported(FNIEnv *env);
jvoid nativeWiFiConnect(FNIEnv *env, jstring ssid, jstring password, jint authMode);
jbool nativeWiFiIsConnected(FNIEnv *env);
jobject nativeWiFiGetAPinfo(FNIEnv *env);
jvoid nativeWiFiDisconnect(FNIEnv *env);
jvoid nativeWiFiSoftAP(FNIEnv *env, jstring ssid, jstring password, jint authMode, jint channel, jint maxConnection);
jvoid nativeWiFiSoftAPdisconnect(FNIEnv *env);
jvoid nativeWiFiStartScan(FNIEnv *env, jbool blocked);
jobjectArray nativeWiFiGetScanResult(FNIEnv *env);
jvoid nativeWiFiStopScan(FNIEnv *env);

static constexpr NativeMethod wifiMethods[] = {
    NATIVE_METHOD("isSupported",      "()Z",                                        nativeWiFiIsSupported),

    NATIVE_METHOD("connect",          "(Ljava/lang/String;Ljava/lang/String;I)V",   nativeWiFiConnect),
    NATIVE_METHOD("isConnected",      "()Z",                                        nativeWiFiIsConnected),
    NATIVE_METHOD("getAPinfo",        "()Lesp/network/AccessPointRecord;",          nativeWiFiGetAPinfo),
    NATIVE_METHOD("disconnect",       "()V",                                        nativeWiFiDisconnect),

    NATIVE_METHOD("softAP",           "(Ljava/lang/String;Ljava/lang/String;III)V", nativeWiFiSoftAP),
    NATIVE_METHOD("softAPdisconnect", "()V",                                        nativeWiFiSoftAPdisconnect),

    NATIVE_METHOD("startScan",        "(Z)V",                                       nativeWiFiStartScan),
    NATIVE_METHOD("getScanResults",   "()[Lesp/network/AccessPointRecord;",         nativeWiFiGetScanResult),
    NATIVE_METHOD("stopScan",         "()V",                                        nativeWiFiStopScan),
};

#endif /* __ESP_NATIVE_WIFI_H */
