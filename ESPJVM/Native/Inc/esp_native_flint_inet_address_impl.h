

#ifndef __ESP_NATIVE_FLINT_INET_ADDRESS_IMPL_H
#define __ESP_NATIVE_FLINT_INET_ADDRESS_IMPL_H

#include "flint_native.h"

jstring NativeFlintInetAddressImpl_GetLocalHostName(FNIEnv *env, jobject obj);
jobjectArray NativeFlintInetAddressImpl_LookupAllHostAddr(FNIEnv *env, jobject obj, jstring hostname);
jstring NativeFlintInetAddressImpl_GetHostByAddr(FNIEnv *env, jobject obj, jbyteArray addr);
jobject NativeFlintInetAddressImpl_AnyLocalAddress(FNIEnv *env, jobject obj);

inline constexpr NativeMethod  flintInetAddressImplMethods[] = {
    NATIVE_METHOD("getLocalHostName",  "()Ljava/lang/String;",                        NativeFlintInetAddressImpl_GetLocalHostName),
    NATIVE_METHOD("lookupAllHostAddr", "(Ljava/lang/String;)[Ljava/net/InetAddress;", NativeFlintInetAddressImpl_LookupAllHostAddr),
    NATIVE_METHOD("getHostByAddr",     "([B)Ljava/lang/String;",                      NativeFlintInetAddressImpl_GetHostByAddr),
    NATIVE_METHOD("anyLocalAddress",   "()Ljava/net/InetAddress;",                    NativeFlintInetAddressImpl_AnyLocalAddress),
};

#endif /* __ESP_NATIVE_FLINT_INET_ADDRESS_IMPL_H */
