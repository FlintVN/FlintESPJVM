
#include <new>
#include "flint.h"
#include <string.h>
#include "esp_wifi.h"
#include "sdkconfig.h"
#include "flint_java_string.h"
#include "flint_system_api.h"
#include "esp_system_native_wifi.h"

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

static void checkParams(FlintExecution &execution, FlintJavaString *ssid, FlintJavaString *password, uint32_t authMode) {
    if(authMode >= sizeof(authValueList)) {
        FlintJavaString &strObj = execution.flint.newString(STR_AND_SIZE("Authentication mode is invalid"));
        throw &execution.flint.newIOException(&strObj);
    }
    if((ssid == NULL) || ((password == NULL) && (authValueList[authMode] != WIFI_AUTH_OPEN))) {
        const char *msg[] = {(ssid == NULL) ? "ssid" : "password", " cannot be null object"};
        FlintJavaString &strObj = execution.flint.newString(msg, LENGTH(msg));
        throw &execution.flint.newNullPointerException(&strObj);
    }

    uint32_t ssidLen = ssid->getLength();
    uint32_t passwordLen = password ? password->getLength() : 0;
    if((ssidLen > 32) || (passwordLen > 64)) {
        const char *msg[] = {(ssidLen > 32) ? "ssid" : "password", " value is invalid"};
        FlintJavaString &strObj = execution.flint.newString(msg, LENGTH(msg));
        throw &execution.flint.newIOException(&strObj);
    }
}

static void checkReturn(FlintExecution &execution, esp_err_t value) {
    if(value != ESP_OK) {
        FlintJavaString &strObj = execution.flint.newString(STR_AND_SIZE("An error occurred while connecting to wifi"));
        throw &execution.flint.newIOException(&strObj);
    }
}

static void nativeIsSupported(FlintExecution &execution) {
    execution.stackPushInt32(1);
}

static void nativeConnect(FlintExecution &execution) {
    uint32_t authMode = execution.stackPopInt32();
    FlintJavaString *password = (FlintJavaString *)execution.stackPopObject();
    FlintJavaString *ssid = (FlintJavaString *)execution.stackPopObject();

    checkParams(execution, ssid, password, authMode);

    uint32_t ssidLen = ssid->getLength();
    uint32_t passwordLen = password ? password->getLength() : 0;
    const char *ssidText = ssid->getText();
    const char *passwordText = password ? password->getText() : 0;
    wifi_config_t wifiConfig = {};
    for(uint8_t i = 0; i < ssidLen; i++)
        wifiConfig.sta.ssid[i] = ssidText[i];
    for(uint8_t i = 0; i < passwordLen; i++)
        wifiConfig.sta.password[i] = passwordText[i];
    wifiConfig.sta.threshold.authmode = authValueList[authMode];
    wifiConfig.sta.pmf_cfg.capable = true;
    wifiConfig.sta.pmf_cfg.required = false;

    execution.flint.lock();
    esp_err_t ret = esp_wifi_set_config(WIFI_IF_STA, &wifiConfig);
    if(ret == ESP_OK)
        ret = esp_wifi_start();
    if(ret == ESP_OK)
        ret = esp_wifi_connect();
    execution.flint.unlock();

    checkReturn(execution, ret);
}

static void nativeIsConnected(FlintExecution &execution) {
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    execution.stackPushInt32((ret == ESP_OK) ? 1 : 0);
}

static FlintJavaObject &createAccessPointRecordObj(FlintExecution &execution, const FlintConstUtf8 &className, wifi_ap_record_t &apRecord) {
    FlintJavaObject &obj = execution.flint.newObject(className);

    /* mac array */
    FlintInt8Array &macArray = execution.flint.newByteArray(6);
    memcpy(macArray.getData(), apRecord.bssid, 6);

    obj.getFields().getFieldObject("mac").object = &macArray;
    obj.getFields().getFieldObject("ssid").object = &execution.flint.newString((char *)apRecord.ssid, strlen((char *)apRecord.ssid));
    obj.getFields().getFieldData32("rssi").value = apRecord.rssi;
    obj.getFields().getFieldData32("authMode").value = apRecord.authmode;

    return obj;
}

static void nativeGetAPinfo(FlintExecution &execution) {
    wifi_ap_record_t apInfo;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&apInfo);
    if(ret == ESP_OK) {
        FlintConstUtf8 &className = execution.flint.getConstUtf8(STR_AND_SIZE("network/AccessPointRecord"));
        execution.stackPushObject(&createAccessPointRecordObj(execution, className, apInfo));
    }
    else
        execution.stackPushObject(0);
}

static void nativeDisconnect(FlintExecution &execution) {
    esp_wifi_disconnect();
}

static void nativeSoftAP(FlintExecution &execution) {
    uint32_t maxConnection = execution.stackPopInt32();
    uint32_t channel = execution.stackPopInt32();
    uint32_t authMode = execution.stackPopInt32();
    FlintJavaString *password = (FlintJavaString *)execution.stackPopObject();
    FlintJavaString *ssid = (FlintJavaString *)execution.stackPopObject();

    checkParams(execution, ssid, password, authMode);

    uint32_t ssidLen = ssid->getLength();
    uint32_t passwordLen = password ? password->getLength() : 0;
    const char *ssidText = ssid->getText();
    const char *passwordText = password ? password->getText() : 0;
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

    execution.flint.lock();
    esp_err_t ret = esp_wifi_set_mode(WIFI_MODE_APSTA);
    if(ret == ESP_OK)
        ret = esp_wifi_set_config(WIFI_IF_AP, &wifiConfig);
    if(ret == ESP_OK)
        ret = esp_wifi_start();
    execution.flint.unlock();

    checkReturn(execution, ret);
}

static void nativeSoftAPdisconnect(FlintExecution &execution) {
    esp_wifi_set_mode(WIFI_MODE_STA);
}

static void nativeStartScan(FlintExecution &execution) {
    int32_t block = execution.stackPopInt32();
    execution.flint.lock();
    esp_err_t ret = esp_wifi_scan_start(NULL, block ? true : false);
    execution.flint.unlock();

    checkReturn(execution, ret);
}

static void nativeGetScanResult(FlintExecution &execution) {
    uint16_t count = 0;
    esp_err_t ret = esp_wifi_scan_get_ap_num(&count);

    checkReturn(execution, ret);
    
    if(count == 0) {
        execution.stackPushObject(NULL);
        return;
    }

    execution.flint.lock();
    try {
        FlintConstUtf8 &className = execution.flint.getConstUtf8(STR_AND_SIZE("network/AccessPointRecord"));
        FlintObjectArray &arrayObj = execution.flint.newObjectArray(className, count);
        FlintJavaObject **data = arrayObj.getData();
        arrayObj.clearData();
        for(uint16_t i = 0; i < count; i++) {
            wifi_ap_record_t apRecords;
            ret = esp_wifi_scan_get_ap_record(&apRecords);
            checkReturn(execution, ret);
            data[i] = &createAccessPointRecordObj(execution, className, apRecords);
        }

        execution.stackPushObject(&arrayObj);
    }
    catch(...) {
        execution.flint.unlock();
        throw;
    }

    execution.flint.unlock();
}

static void nativeStopScan(FlintExecution &execution) {
    esp_wifi_scan_stop();
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x0B\x00\x2B\x6D""isSupported",      "\x03\x00\x91\x9C""()Z",                                        nativeIsSupported),

    NATIVE_METHOD("\x07\x00\x73\x4F""connect",          "\x28\x00\x84\x65""(Ljava/lang/String;Ljava/lang/String;I)V",   nativeConnect),
    NATIVE_METHOD("\x0B\x00\x07\x5C""isConnected",      "\x03\x00\x91\x9C""()Z",                                        nativeIsConnected),
    NATIVE_METHOD("\x09\x00\x51\xFD""getAPinfo",        "\x1D\x00\x50\xDF""()Lnetwork/AccessPointRecord;",              nativeGetAPinfo),
    NATIVE_METHOD("\x0A\x00\xDF\x40""disconnect",       "\x03\x00\x91\x99""()V",                                        nativeDisconnect),

    NATIVE_METHOD("\x06\x00\x4E\x10""softAP",           "\x2A\x00\xED\x0C""(Ljava/lang/String;Ljava/lang/String;III)V", nativeSoftAP),
    NATIVE_METHOD("\x10\x00\x71\xA1""softAPdisconnect", "\x03\x00\x91\x99""()V",                                        nativeSoftAPdisconnect),

    NATIVE_METHOD("\x09\x00\x01\xD3""startScan",        "\x04\x00\x49\xC6""(Z)V",                                       nativeStartScan),
    NATIVE_METHOD("\x0E\x00\x13\x81""getScanResults",   "\x1E\x00\x79\x2F""()[Lnetwork/AccessPointRecord;",             nativeGetScanResult),
    NATIVE_METHOD("\x08\x00\xCD\x70""stopScan",         "\x03\x00\x91\x99""()V",                                        nativeStopScan),
};

const FlintNativeClass WIFI_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x0C\x00\xE6\x3F""network/WiFi", methods);
