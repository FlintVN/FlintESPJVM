

#ifndef __ESP_NATIVE_FLINT_SOCKET_IMPL_H
#define __ESP_NATIVE_FLINT_SOCKET_IMPL_H

#include "flint_native.h"

int32_t NativeFlintSocketImpl_GetSock(FNIEnv *env, jobject socketObj, bool throwable);

jvoid NativeFlintSocketImpl_SocketCreate(FNIEnv *env, jobject obj);
jvoid NativeFlintSocketImpl_SocketConnect(FNIEnv *env, jobject obj, jobject address, jint port);
jvoid NativeFlintSocketImpl_SocketBind(FNIEnv *env, jobject obj, jobject address, jint port);
jvoid NativeFlintSocketImpl_SocketListen(FNIEnv *env, jobject obj, jint count);
jvoid NativeFlintSocketImpl_SocketAccept(FNIEnv *env, jobject obj, jobject s);
jint NativeFlintSocketImpl_SocketAvailable(FNIEnv *env, jobject obj);
jvoid NativeFlintSocketImpl_SocketClose(FNIEnv *env, jobject obj);
jvoid NativeFlintSocketImpl_InitProto(FNIEnv *env);
jvoid NativeFlintSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint cmd, jbool on, jobject value);
jint NativeFlintSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt);

inline constexpr NativeMethod flintSocketImplMethods[] = {
    NATIVE_METHOD("socketCreate",    "()V",                        NativeFlintSocketImpl_SocketCreate),
    NATIVE_METHOD("socketConnect",   "(Ljava/net/InetAddress;I)V", NativeFlintSocketImpl_SocketConnect),
    NATIVE_METHOD("socketBind",      "(Ljava/net/InetAddress;I)V", NativeFlintSocketImpl_SocketBind),
    NATIVE_METHOD("socketListen",    "(I)V",                       NativeFlintSocketImpl_SocketListen),
    NATIVE_METHOD("socketAccept",    "(Ljava/net/SocketImpl;)V",   NativeFlintSocketImpl_SocketAccept),
    NATIVE_METHOD("socketAvailable", "()I",                        NativeFlintSocketImpl_SocketAvailable),
    NATIVE_METHOD("socketClose",     "()V",                        NativeFlintSocketImpl_SocketClose),
    NATIVE_METHOD("initProto",       "()V",                        NativeFlintSocketImpl_InitProto),
    NATIVE_METHOD("socketSetOption", "(IZLjava/lang/Object;)V",    NativeFlintSocketImpl_SocketSetOption),
    NATIVE_METHOD("socketGetOption", "(I)I",                       NativeFlintSocketImpl_SocketGetOption),
};

#endif /* __ESP_NATIVE_FLINT_SOCKET_IMPL_H */
