
#include <new>
#include "flint.h"
#include <string.h>
#include "esp_wifi.h"
#include "sdkconfig.h"
#include "flint_java_string.h"
#include "flint_system_api.h"
#include "esp_native_wifi.h"

static const wifi_auth_mode_t authValueList[] = {
    WIFI_AUTH_OPEN,
    WIFI_AUTH_WEP,
    WIFI_AUTH_WPA_PSK,
    WIFI_AUTH_WPA2_PSK,
    WIFI_AUTH_WPA_WPA2_PSK,
    WIFI_AUTH_ENTERPRISE,
    WIFI_AUTH_WPA2_ENTERPRISE,
    WIFI_AUTH_WPA3_PSK,
    WIFI_AUTH_WPA2_WPA3_PSK,
    WIFI_AUTH_WAPI_PSK,
    WIFI_AUTH_OWE,
    WIFI_AUTH_WPA3_ENT_192,
    WIFI_AUTH_WPA3_EXT_PSK,
    WIFI_AUTH_WPA3_EXT_PSK_MIXED_MODE,
    WIFI_AUTH_DPP,
};

static bool CheckParams(FNIEnv *env, jstring ssid, jstring password, uint32_t authMode) {
    if(authMode >= sizeof(authValueList)) {
        env->throwNew(env->findClass("java/io/IOException"), "Authentication mode is invalid");
        return false;
    }
    if((ssid == NULL) || ((password == NULL) && (authValueList[authMode] != WIFI_AUTH_OPEN))) {
        if(ssid == NULL) {
            env->throwNew(env->findClass("java/lang/NullPointerException"), "ssid cannot be null object");
            return false;
        }
        else {
            env->throwNew(env->findClass("java/lang/NullPointerException"), "password cannot be null object");
            return false;
        }
    }

    uint32_t ssidLen = ssid->getLength();
    uint32_t passwordLen = password ? password->getLength() : 0;
    if((ssidLen > 32) || (passwordLen > 64)) {
        if(ssidLen > 32) {
            env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "ssid value is invalid");
            return false;
        }
        else {
            env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "password value is invalid");
            return false;
        }
    }

    return true;
}

static bool CheckReturn(FNIEnv *env, esp_err_t value) {
    if(value != ESP_OK) {
        env->throwNew(env->findClass("java/io/IOException"), "An error occurred while connecting to wifi");
        return false;
    }
    return true;
}

jbool NativeWiFi_IsSupported(FNIEnv *env) {
    return true;
}

jvoid NativeWiFi_Connect(FNIEnv *env, jstring ssid, jstring password, jint authMode) {
    if(!CheckParams(env, ssid, password, authMode)) return;

    uint32_t ssidLen = ssid->getLength();
    uint32_t passwordLen = password ? password->getLength() : 0;
    const char *ssidText = ssid->getAscii();
    const char *passwordText = password ? password->getAscii() : 0;
    wifi_config_t wifiConfig = {};
    for(uint8_t i = 0; i < ssidLen; i++)
        wifiConfig.sta.ssid[i] = ssidText[i];
    for(uint8_t i = 0; i < passwordLen; i++)
        wifiConfig.sta.password[i] = passwordText[i];
    wifiConfig.sta.threshold.authmode = authValueList[authMode];
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;

    Flint::lock();
    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if(ret == ESP_OK)
        ret = esp_wifi_start();
    if(ret == ESP_OK)
        ret = esp_wifi_connect();
    Flint::unlock();

    CheckReturn(env, ret);
}

jbool NativeWiFi_IsConnected(FNIEnv *env) {
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    return (ret == ESP_OK) ? true : false;
}

static jobject createAccessPointRecordObj(FNIEnv *env, wifi_ap_record_t *apRecord) {
    jobject aprObj = env->newObject(env->findClass("flint/net/AccessPointRecord"));
    if(aprObj == NULL) return NULL;

    /* mac array */
    jbyteArray macArray = env->newByteArray(6);
    if(macArray == NULL) return NULL;
    memcpy(macArray->getData(), apRecord->bssid, 6);

    aprObj->getField(env->exec, "mac")->setObj(macArray);
    jstring ssid = env->newString((char *)apRecord->ssid);
    if(ssid == NULL) {
        env->freeObject(macArray);
        return NULL;
    }
    aprObj->getField(env->exec, "ssid")->setObj(ssid);
    aprObj->getField(env->exec, "rssi")->setInt32(apRecord->rssi);
    aprObj->getField(env->exec, "authMode")->setInt32(apRecord->authmode);

    return aprObj;
}

jobject NativeWiFi_GetAPinfo(FNIEnv *env) {
    wifi_ap_record_t apInfo;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&apInfo);
    if(ret == ESP_OK) {
        jobject obj = createAccessPointRecordObj(env, &apInfo);
        if(obj == NULL) return NULL;
        return obj;
    }
    return NULL;
}

jvoid NativeWiFi_Disconnect(FNIEnv *env) {
    esp_wifi_disconnect();
}

jvoid NativeWiFi_SoftAP(FNIEnv *env, jstring ssid, jstring password, jint authMode, jint channel, jint maxConnection) {
    if(!CheckParams(env, ssid, password, authMode)) return;

    uint32_t ssidLen = ssid->getLength();
    uint32_t passwordLen = password ? password->getLength() : 0;
    const char *ssidText = ssid->getAscii();
    const char *passwordText = password ? password->getAscii() : 0;
    wifi_config_t wifiConfig = {};
    wifiConfig.ap.ssid_len = ssidLen;
    wifiConfig.ap.channel = channel;
    wifiConfig.ap.max_connection = maxConnection;
    wifiConfig.ap.authmode = authValueList[authMode];
    wifiConfig.ap.pmf_cfg.required = false;
    for(uint8_t i = 0; i < ssidLen; i++)
        wifiConfig.ap.ssid[i] = ssidText[i];
    for(uint8_t i = 0; i < passwordLen; i++)
        wifiConfig.ap.password[i] = passwordText[i];

    Flint::lock();
    esp_err_t ret = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if(ret == ESP_OK)
        ret = esp_wifi_set_config(WIFI_IF_AP, &wifiConfig);
    if(ret == ESP_OK)
        ret = esp_wifi_start();
    Flint::unlock();

    CheckReturn(env, ret);
}

jvoid NativeWiFi_SoftAPdisconnect(FNIEnv *env) {
    esp_wifi_set_mode(WIFI_MODE_STA);
}

jvoid NativeWiFi_StartScan(FNIEnv *env, jbool blocked) {
    Flint::lock();
    esp_err_t ret = esp_wifi_scan_start(NULL, blocked ? true : false);
    Flint::unlock();
    CheckReturn(env, ret);
}

jobjectArray NativeWiFi_GetScanResult(FNIEnv *env) {
    uint16_t count = 0;
    esp_err_t ret = esp_wifi_scan_get_ap_num(&count);

    if(!CheckReturn(env, ret)) return NULL;

    if(count == 0) return NULL;

    Flint::lock();
    jobjectArray arrayObj = env->newObjectArray(env->findClass("flint/net/AccessPointRecord"), count);
    if(arrayObj == NULL) return NULL;
    arrayObj->clearData();
    JObject **data = arrayObj->getData();
    for(uint16_t i = 0; i < count; i++) {
        wifi_ap_record_t apRecords;
        ret = esp_wifi_scan_get_ap_record(&apRecords);
        if(!CheckReturn(env, ret)) {
            Flint::unlock();
            return NULL;
        }
        jobject aprObj = createAccessPointRecordObj(env, &apRecords);
        if(aprObj == NULL) {
            Flint::unlock();
            while(i) env->freeObject(data[--i]);
            env->freeObject(arrayObj);
            return NULL;
        }
        data[i] = aprObj;
    }

    Flint::unlock();
    return arrayObj;
}

jvoid NativeWiFi_StopScan(FNIEnv *env) {
    esp_wifi_scan_stop();
}
