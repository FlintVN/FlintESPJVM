
#include <string.h>
#include "flint.h"
// #include "soc/i2c_pins.h"
#include "driver/i2c_master.h"
#include "flint_system_api.h"
#include "esp_system_native_pin.h"
#include "esp_system_native_i2c_master.h"

typedef class : public JObject {
public:
    String getI2cName() { return (jstring)getFieldByIndex(0)->getObj(); }
    int getI2cId() { return getFieldByIndex(1)->getInt32(); }
    int getSpeed() { return getFieldByIndex(2)->getInt32(); }
    int getSlaveAddr() { return getFieldByIndex(3)->getInt32(); }
    int getSda() { return getFieldByIndex(4)->getInt32(); }
    int getScl() { return getFieldByIndex(5)->getInt32(); }

    void setI2cId(int32_t val) { getFieldByIndex(1)->setInt32(val); }
    void setSpeed(int32_t val) { getFieldByIndex(2)->setInt32(val); }
    void setSda(int32_t val) { getFieldByIndex(4)->setInt32(val); }
    void setScl(int32_t val) { getFieldByIndex(5)->setInt32(val); }
} *I2cMasterObject;


void NativeI2cMaster_Reset(void) {

}

static int32_t getI2cId(FNIEnv *env, jstring i2cName) {
    if(i2cName == NULL) {
        env->throwNew(env->findClass("java/io/NullPointerException"), "I2C name cannot be null");
        return -1;
    }
    // TODO
}

static bool checkI2cMasterPin(FNIEnv *env, I2cMasterObject i2cObj) {
    const char *msg;
    int32_t sda = i2cObj->getSda();
    int32_t scl = i2cObj->getScl();

    if(sda < 0 || scl < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "SDA and SCL must be specified");
        return -1;
    }
    else if((msg = NativePin_CheckPin(sda)) != NULL || (msg = NativePin_CheckPin(scl)) != NULL) {
        env->throwNew(env->findClass("java/io/IOException"), msg);
        return -1;
    }
    else if(sda == scl) {
        env->throwNew(env->findClass("java/io/NullPointerException"), "SDA and SCL cannot have same value");
        return -1;
    }
}

jobject nativeI2cMasterOpen(FNIEnv *env, jobject obj) {
    I2cMasterObject i2cObj = (I2cMasterObject)obj;
    int32_t i2cId = getI2cId(env, i2cObj->getI2cName());
    if(i2cId == -1) return obj;

    i2cObj->setI2cId(i2cId);
    if(i2cObj->getSda() < 0) i2cObj->setSda(-1);
    if(i2cObj->getScl() < 0) i2cObj->setScl(-1);

    checkI2cMasterPin(env, i2cObj);
}

jboolean nativeI2cMasterIsOpen(FNIEnv *env, jobject obj) {
    
}

jint nativeI2cMasterGetSpeed(FNIEnv *env, jobject obj) {

}

jint nativeI2cMasterReadByte(FNIEnv *env, jobject obj) {

}

jint nativeI2cMasterRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {

}

jvoid nativeI2cMasterWriteByte(FNIEnv *env, jobject obj, jint b) {

}

jvoid nativeI2cMasterWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint count) {

}

jvoid nativeI2cMasterWriteMemByte(FNIEnv *env, jobject obj, jint memAddr, jint b) {

}

jvoid nativeI2cMasterWriteMem(FNIEnv *env, jobject obj, jint memAddr, jbyteArray buff, jint off, jint count) {

}

jvoid nativeI2cMasterClose(FNIEnv *env, jobject obj) {

}
