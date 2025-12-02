
#include <string.h>
#include "flint.h"
#include "driver/i2c.h"
#include "flint_system_api.h"
#include "esp_native_common.h"
#include "esp_native_pin.h"
#include "esp_native_i2c_master.h"

typedef class : public JObject {
public:
    jstring getI2cName() { return (jstring)getFieldByIndex(0)->getObj(); }
    int32_t getI2cId() { return getFieldByIndex(1)->getInt32(); }
    int32_t getSpeed() { return getFieldByIndex(2)->getInt32(); }
    int32_t getDevAddr() { return getFieldByIndex(3)->getInt32(); }
    int32_t getSda() { return getFieldByIndex(4)->getInt32(); }
    int32_t getScl() { return getFieldByIndex(5)->getInt32(); }

    void setI2cId(int32_t val) { getFieldByIndex(1)->setInt32(val); }
    void setSpeed(int32_t val) { getFieldByIndex(2)->setInt32(val); }
    void setSda(int32_t val) { getFieldByIndex(4)->setInt32(val); }
    void setScl(int32_t val) { getFieldByIndex(5)->setInt32(val); }
} *I2cMasterObject;

static jobject i2cHolder[I2C_NUM_MAX] = {};

static bool NativeI2cMaster_IsOpen(int32_t i2cId) {
    if(0 <= i2cId || i2cId < LENGTH(i2cHolder))
        return i2cHolder[i2cId] != NULL ? true : false;
    return false;
}

static void NativeI2cMaster_Close(int32_t i2cId) {
    i2c_driver_delete((i2c_port_t)i2cId);
    i2cHolder[i2cId] = NULL;
}

void NativeI2cMaster_Reset(void) {
    for(int8_t i = 0; i < LENGTH(i2cHolder); i++)
        NativeI2cMaster_Close(i);
}

static int32_t getI2cId(FNIEnv *env, jstring i2cName) {
    if(i2cName == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"), "I2C name cannot be null");
        return -1;
    }
    if(i2cName->compareTo("I2C0", strlen("I2C0")) == 0)
        return I2C_NUM_0;
#if SOC_HP_I2C_NUM >= 2
    else if(i2cName->compareTo("I2C1", strlen("I2C1")) == 0)
        return I2C_NUM_1;
#endif
#if SOC_LP_I2C_NUM >= 1
    else if(i2cName->compareTo("LPI2C0", strlen("LPI2C0")) == 0)
        return LP_I2C_NUM_0;
#endif
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid I2C name");
        return -1;
    }
}

static bool checkI2cMasterPin(FNIEnv *env, I2cMasterObject i2cObj) {
    const char *msg;
    int32_t sda = i2cObj->getSda();
    int32_t scl = i2cObj->getScl();

    if(sda < 0 || scl < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "SDA and SCL must be specified");
        return false;
    }
    else if((msg = NativePin_CheckPin(sda)) != NULL || (msg = NativePin_CheckPin(scl)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return false;
    }
    else if(sda == scl) {
        env->throwNew(env->findClass("java/io/IOException"), "SDA and SCL cannot have same value");
        return false;
    }
    return true;
}

static bool checkPrecondition(FNIEnv *env, I2cMasterObject i2cObj) {
    int32_t i2cId = i2cObj->getI2cId();
    if(!NativeI2cMaster_IsOpen(i2cId)) {
        env->throwNew(env->findClass("java/io/IOException"), "I2C has not been opened yet");
        return false;
    }
    else if(i2cHolder[i2cId] != i2cObj) {
        env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
        return false;
    }
    int32_t devAddr = i2cObj->getDevAddr();
    if(devAddr < 0 || devAddr > 0x7F) {
        env->throwNew(env->findClass("java/io/IOException"), "Device address is not specified or invalid, the value should be between 0x00 and 0x7F");
        return false;
    }
    return true;
}

jobject nativeI2cMasterOpen(FNIEnv *env, jobject obj) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    int32_t i2cId = getI2cId(env, i2cObj->getI2cName());
    if(i2cId == -1) return obj;

    if(i2cObj->getSda() < 0) i2cObj->setSda(-1);
    if(i2cObj->getScl() < 0) i2cObj->setScl(-1);
    if(i2cObj->getSpeed() < 0) i2cObj->setSpeed(400000);

    if(!checkI2cMasterPin(env, i2cObj)) return obj;

    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = i2cObj->getSda();
    conf.scl_io_num = i2cObj->getScl();
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = i2cObj->getSpeed();
    esp_err_t err = i2c_param_config((i2c_port_t)i2cId, &conf);
    if(err == ESP_OK) err = i2c_driver_install((i2c_port_t)i2cId, conf.mode, 0, 0, 0);
    if(err == ESP_OK)
        i2cHolder[i2cId] = i2cObj;
    else {
        env->throwNew(env->findClass("java/io/IOException"), "Error while opening I2C port");
        return obj;
    }

    i2cObj->setI2cId(i2cId);
    return obj;
}

jbool nativeI2cMasterIsOpen(FNIEnv *env, jobject obj) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    int32_t i2cId = i2cObj->getI2cId();
    if(NativeI2cMaster_IsOpen(i2cId))
        return (i2cHolder[i2cId] == i2cObj) ? true : false;
    return false;
}

jint nativeI2cMasterGetSpeed(FNIEnv *env, jobject obj) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    return i2cObj->getSpeed();
}

jint nativeI2cMasterReadByte(FNIEnv *env, jobject obj) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return -1;

    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1) | I2C_MASTER_READ, true);
    if(err == ESP_OK) err = i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while reading data");
    return data;
}

jint nativeI2cMasterRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1) | I2C_MASTER_READ, true);
    if(err == ESP_OK) err = i2c_master_read(cmd, (uint8_t *)&b->getData()[off], count, I2C_MASTER_LAST_NACK);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while reading data");
    return count;
}

jvoid nativeI2cMasterWriteByte(FNIEnv *env, jobject obj, jint b) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1), true);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (uint8_t)b, true);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while writing data");
}

jvoid nativeI2cMasterWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return;
    if(!CheckArrayIndexSize(env, b, off, count)) return;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1), true);
    if(err == ESP_OK) err = i2c_master_write(cmd, (uint8_t *)&b->getData()[off], count, true);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while writing data");
}

jint nativeI2cMasterReadMemByte(FNIEnv *env, jobject obj, jint memAddr) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return -1;

    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1), true);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, memAddr, true);
    if(err == ESP_OK) err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1) | I2C_MASTER_READ, true);
    if(err == ESP_OK) err = i2c_master_read_byte(cmd, &data, I2C_MASTER_NACK);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while reading data");
    return data;
}

jint nativeI2cMasterReadMem(FNIEnv *env, jobject obj, jint memAddr, jbyteArray b, jint off, jint count) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return 0;
    if(!CheckArrayIndexSize(env, b, off, count)) return 0;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1), true);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, memAddr, true);
    if(err == ESP_OK) err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1) | I2C_MASTER_READ, true);
    if(err == ESP_OK) err = i2c_master_read(cmd, (uint8_t *)&b->getData()[off], count, I2C_MASTER_LAST_NACK);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while reading data");
    return count;
}

jvoid nativeI2cMasterWriteMemByte(FNIEnv *env, jobject obj, jint memAddr, jint b) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1), true);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (uint8_t)memAddr, true);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (uint8_t)b, true);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while writing data");
}

jvoid nativeI2cMasterWriteMem(FNIEnv *env, jobject obj, jint memAddr, jbyteArray b, jint off, jint count) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    if(!checkPrecondition(env, i2cObj)) return;
    if(!CheckArrayIndexSize(env, b, off, count)) return;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t err = i2c_master_start(cmd);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (i2cObj->getDevAddr() << 1), true);
    if(err == ESP_OK) err = i2c_master_write_byte(cmd, (uint8_t)memAddr, true);
    if(err == ESP_OK) err = i2c_master_write(cmd, (uint8_t *)&b->getData()[off], count, true);
    if(err == ESP_OK) err = i2c_master_stop(cmd);
    if(err == ESP_OK) err = i2c_master_cmd_begin((i2c_port_t)i2cObj->getI2cId(), cmd, pdMS_TO_TICKS(20));
    i2c_cmd_link_delete(cmd);

    if(err != ESP_OK) env->throwNew(env->findClass("java/io/IOException"), "Error while writing data");
}

jvoid nativeI2cMasterClose(FNIEnv *env, jobject obj) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    int32_t i2cId = i2cObj->getI2cId();
    if(NativeI2cMaster_IsOpen(i2cId)) {
        if(i2cHolder[i2cId] != i2cObj) {
            env->throwNew(env->findClass("java/io/IOException"), "Access is denied");
            return;
        }
        NativeI2cMaster_Close(i2cId);
    }
    i2cObj->setI2cId(-1);
}
