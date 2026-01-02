

#ifndef __ESP_FLINT_JAVA_INET_ADDRESS_H
#define __ESP_FLINT_JAVA_INET_ADDRESS_H

#include "flint.h"
#include "flint_java_object.h"
#include "flint_java_string.h"

class InetAddress : public JObject {
public:
    jstring getHostName() { return (jstring)getFieldByIndex(0)->getObj(); }
    void setHostName(jstring name) { getFieldByIndex(0)->setObj(name); }

    jint getFamily() { return getFieldByIndex(1)->getInt32(); }
    void setFamily(int32_t val) { return getFieldByIndex(1)->setInt32(val); }
};

class Inet4Address : public InetAddress {
public:
    jint getAddress() { return getFieldByIndex(2)->getInt32(); }
    void setAddress(int32_t address) { getFieldByIndex(2)->setInt32(address); }
};

class Inet6Address : public InetAddress {
public:
    jbyteArray getAddress() { return (jbyteArray)getFieldByIndex(2)->getObj(); }
    void setAddress(jbyteArray address) { getFieldByIndex(2)->setObj(address); }

    jint getScopeId() { return getFieldByIndex(3)->getInt32(); }
    void setScopeId(jint val) { return getFieldByIndex(3)->setInt32(val); }

    jbool getScopeIdSet() { return getFieldByIndex(4)->getInt32(); }
    void setScopeIdSet(jbool val) { return getFieldByIndex(4)->setInt32(val); }
};

#endif /* __ESP_FLINT_JAVA_INET_ADDRESS_H */
