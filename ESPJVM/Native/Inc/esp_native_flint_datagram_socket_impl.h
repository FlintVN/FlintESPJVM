

#ifndef __ESP_NATIVE_FLINT_DATAGRAM_SOCKET_IMPL_H
#define __ESP_NATIVE_FLINT_DATAGRAM_SOCKET_IMPL_H

#include "flint_native.h"

jvoid NativeFlintDatagramSocketImpl_Bind(FNIEnv *env, jobject obj, jint lport, jobject laddr);
jvoid NativeFlintDatagramSocketImpl_Send(FNIEnv *env, jobject obj, jobject p);
jint NativeFlintDatagramSocketImpl_Peek(FNIEnv *env, jobject obj, jobject i);
jvoid NativeFlintDatagramSocketImpl_Receive(FNIEnv *env, jobject obj, jobject p);
jvoid NativeFlintDatagramSocketImpl_SetTTL(FNIEnv *env, jobject obj, jbyte ttl);
jbyte NativeFlintDatagramSocketImpl_GetTTL(FNIEnv *env, jobject obj);
jvoid NativeFlintDatagramSocketImpl_Join(FNIEnv *env, jobject obj, jobject inetaddr);
jvoid NativeFlintDatagramSocketImpl_Leave(FNIEnv *env, jobject obj, jobject inetaddr);
jvoid NativeFlintDatagramSocketImpl_DatagramSocketCreate(FNIEnv *env, jobject obj);
jvoid NativeFlintDatagramSocketImpl_DatagramSocketClose(FNIEnv *env, jobject obj);
jvoid NativeFlintDatagramSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint opt, jobject val);
jobject NativeFlintDatagramSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt);

inline constexpr NativeMethod flintDatagramSocketImplMethods[] = {
    NATIVE_METHOD("bind",                 "(ILjava/net/InetAddress;)V",   NativeFlintDatagramSocketImpl_Bind),
    NATIVE_METHOD("send",                 "(Ljava/net/DatagramPacket;)V", NativeFlintDatagramSocketImpl_Send),
    NATIVE_METHOD("peek",                 "(Ljava/net/InetAddress;)I",    NativeFlintDatagramSocketImpl_Peek),
    NATIVE_METHOD("receive",              "(Ljava/net/DatagramPacket;)V", NativeFlintDatagramSocketImpl_Receive),
    NATIVE_METHOD("setTTL",               "(B)V",                         NativeFlintDatagramSocketImpl_SetTTL),
    NATIVE_METHOD("getTTL",               "()B",                          NativeFlintDatagramSocketImpl_GetTTL),
    NATIVE_METHOD("join",                 "(Ljava/net/InetAddress;)V",    NativeFlintDatagramSocketImpl_Join),
    NATIVE_METHOD("leave",                "(Ljava/net/InetAddress;)V",    NativeFlintDatagramSocketImpl_Leave),
    NATIVE_METHOD("datagramSocketCreate", "()V",                          NativeFlintDatagramSocketImpl_DatagramSocketCreate),
    NATIVE_METHOD("datagramSocketClose",  "()V",                          NativeFlintDatagramSocketImpl_DatagramSocketClose),
    NATIVE_METHOD("socketSetOption",      "(ILjava/lang/Object;)V",       NativeFlintDatagramSocketImpl_SocketSetOption),
    NATIVE_METHOD("socketGetOption",      "(I)Ljava/lang/Object;",        NativeFlintDatagramSocketImpl_SocketGetOption),
};

#endif /* __ESP_NATIVE_FLINT_DATAGRAM_SOCKET_IMPL_H */
