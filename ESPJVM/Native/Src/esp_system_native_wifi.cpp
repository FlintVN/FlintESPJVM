
#include <new>
#include "flint.h"
#include <string.h>
#include "esp_wifi.h"
#include "sdkconfig.h"
#include "flint_java_string.h"
#include "flint_system_api.h"
#include "esp_system_native_wifi.h"
#include "flint_throw_support.h"

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

static FlintError checkParams(FlintExecution &execution, FlintJavaString *ssid, FlintJavaString *password, uint32_t authMode) {
    if(authMode >= sizeof(authValueList))
        return throwIOException(execution, "Authentication mode is invalid");
    if((ssid == NULL) || ((password == NULL) && (authValueList[authMode] != WIFI_AUTH_OPEN))) {
        if(ssid == NULL)
            return throwNullPointerException(execution, "ssid cannot be null object");
        else
            return throwNullPointerException(execution, "password cannot be null object");
    }

    uint32_t ssidLen = ssid->getLength();
    uint32_t passwordLen = password ? password->getLength() : 0;
    if((ssidLen > 32) || (passwordLen > 64)) {
        if(ssidLen > 32)
            return throwIllegalArgumentException(execution, "ssid value is invalid");
        else
            return throwIllegalArgumentException(execution, "password value is invalid");
    }

    return ERR_OK;
}

static FlintError checkReturn(FlintExecution &execution, esp_err_t value) {
    if(value != ESP_OK)
        return throwIOException(execution, "An error occurred while connecting to wifi");
    return ERR_OK;
}

static FlintError nativeIsSupported(FlintExecution &execution) {
    execution.stackPushInt32(1);
    return ERR_OK;
}

static FlintError nativeConnect(FlintExecution &execution) {
    uint32_t authMode = execution.stackPopInt32();
    FlintJavaString *password = (FlintJavaString *)execution.stackPopObject();
    FlintJavaString *ssid = (FlintJavaString *)execution.stackPopObject();

    RETURN_IF_ERR(checkParams(execution, ssid, password, authMode));

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

    return checkReturn(execution, ret);
}

static FlintError nativeIsConnected(FlintExecution &execution) {
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    execution.stackPushInt32((ret == ESP_OK) ? 1 : 0);
    return ERR_OK;
}

static FlintError createAccessPointRecordObj(FlintExecution &execution, const FlintConstUtf8 &className, wifi_ap_record_t &apRecord, FlintJavaObject *&aprObj) {
    FlintError err = execution.flint.newObject(className, aprObj);
    if(err != ERR_OK)
        return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)aprObj);

    /* mac array */
    FlintInt8Array *macArray;
    RETURN_IF_ERR(execution.flint.newByteArray(6, macArray));
    memcpy(macArray->getData(), apRecord.bssid, 6);

    FlintFieldsData &fileds = aprObj->getFields();
    fileds.getFieldObject("mac")->object = macArray;
    FlintJavaString *ssid;
    err = execution.flint.newString((char *)apRecord.ssid, ssid);
    if(err == ERR_OK) {
        execution.flint.freeObject(*macArray);
        return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)ssid);
    }
    fileds.getFieldObject("ssid")->object = ssid;
    fileds.getFieldData32("rssi")->value = apRecord.rssi;
    fileds.getFieldData32("authMode")->value = apRecord.authmode;

    return ERR_OK;
}

static FlintError nativeGetAPinfo(FlintExecution &execution) {
    wifi_ap_record_t apInfo;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&apInfo);
    if(ret == ESP_OK) {
        FlintConstUtf8 &className = execution.flint.getConstUtf8(STR_AND_SIZE("esp/network/AccessPointRecord"));
        FlintJavaObject *obj;
        RETURN_IF_ERR(createAccessPointRecordObj(execution, className, apInfo, obj));
        execution.stackPushObject(obj);
    }
    else
        execution.stackPushObject(0);
    return ERR_OK;
}

static FlintError nativeDisconnect(FlintExecution &execution) {
    esp_wifi_disconnect();
    return ERR_OK;
}

static FlintError nativeSoftAP(FlintExecution &execution) {
    uint32_t maxConnection = execution.stackPopInt32();
    uint32_t channel = execution.stackPopInt32();
    uint32_t authMode = execution.stackPopInt32();
    FlintJavaString *password = (FlintJavaString *)execution.stackPopObject();
    FlintJavaString *ssid = (FlintJavaString *)execution.stackPopObject();

    RETURN_IF_ERR(checkParams(execution, ssid, password, authMode));

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

    return checkReturn(execution, ret);
    return ERR_OK;
}

static FlintError nativeSoftAPdisconnect(FlintExecution &execution) {
    esp_wifi_set_mode(WIFI_MODE_STA);
    return ERR_OK;
}

static FlintError nativeStartScan(FlintExecution &execution) {
    int32_t block = execution.stackPopInt32();
    execution.flint.lock();
    esp_err_t ret = esp_wifi_scan_start(NULL, block ? true : false);
    execution.flint.unlock();

    return checkReturn(execution, ret);
}

static FlintError nativeGetScanResult(FlintExecution &execution) {
    uint16_t count = 0;
    esp_err_t ret = esp_wifi_scan_get_ap_num(&count);

    RETURN_IF_ERR(checkReturn(execution, ret));
    
    if(count == 0) {
        execution.stackPushObject(NULL);
        return ERR_OK;
    }

    execution.flint.lock();
    FlintConstUtf8 &className = execution.flint.getConstUtf8(STR_AND_SIZE("esp/network/AccessPointRecord"));
    FlintObjectArray *arrayObj;
    FlintError err = execution.flint.newObjectArray(className, count, arrayObj);
    if(err != ERR_OK) {
        execution.flint.unlock();
        return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)arrayObj);
    }
    FlintJavaObject **data = arrayObj->getData();
    arrayObj->clearData();
    for(uint16_t i = 0; i < count; i++) {
        wifi_ap_record_t apRecords;
        ret = esp_wifi_scan_get_ap_record(&apRecords);
        FlintError err = checkReturn(execution, ret);
        if(err != ERR_OK) {
            execution.flint.unlock();
            return err;
        }
        err = createAccessPointRecordObj(execution, className, apRecords, data[i]);
        if(err != ERR_OK) {
            execution.flint.unlock();
            return err;
        }
    }

    execution.stackPushObject(arrayObj);

    execution.flint.unlock();
    return ERR_OK;
}

static FlintError nativeStopScan(FlintExecution &execution) {
    esp_wifi_scan_stop();
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x0B\x00\x2B\x6D""isSupported",      "\x03\x00\x91\x9C""()Z",                                        nativeIsSupported),

    NATIVE_METHOD("\x07\x00\x73\x4F""connect",          "\x28\x00\x84\x65""(Ljava/lang/String;Ljava/lang/String;I)V",   nativeConnect),
    NATIVE_METHOD("\x0B\x00\x07\x5C""isConnected",      "\x03\x00\x91\x9C""()Z",                                        nativeIsConnected),
    NATIVE_METHOD("\x09\x00\x51\xFD""getAPinfo",        "\x21\x00\x53\x56""()Lesp/network/AccessPointRecord;",          nativeGetAPinfo),
    NATIVE_METHOD("\x0A\x00\xDF\x40""disconnect",       "\x03\x00\x91\x99""()V",                                        nativeDisconnect),

    NATIVE_METHOD("\x06\x00\x4E\x10""softAP",           "\x2A\x00\xED\x0C""(Ljava/lang/String;Ljava/lang/String;III)V", nativeSoftAP),
    NATIVE_METHOD("\x10\x00\x71\xA1""softAPdisconnect", "\x03\x00\x91\x99""()V",                                        nativeSoftAPdisconnect),

    NATIVE_METHOD("\x09\x00\x01\xD3""startScan",        "\x04\x00\x49\xC6""(Z)V",                                       nativeStartScan),
    NATIVE_METHOD("\x0E\x00\x13\x81""getScanResults",   "\x22\x00\x5B\x39""()[Lesp/network/AccessPointRecord;",         nativeGetScanResult),
    NATIVE_METHOD("\x08\x00\xCD\x70""stopScan",         "\x03\x00\x91\x99""()V",                                        nativeStopScan),
};

const FlintNativeClass WIFI_CLASS = NATIVE_CLASS(*(const FlintConstUtf8 *)"\x10\x00\xFE\x4C""esp/network/WiFi", methods);
