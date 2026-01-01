

#include "esp_socket.h"
#include "flint_java_object.h"
#include "esp_native_common.h"
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_input_stream.h"

using namespace FlintAPI::System;

extern int32_t NativeFlintSocketImpl_GetSock(FNIEnv *env, jobject socketObj, bool throwable);

jint NativeFlintSocketInputStream_SocketRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return -1;

    if(!CheckArrayIndexSize(env, b, off, len)) return -1;

    jobject impl = obj->getField(env->exec, "impl")->getObj();
    int32_t timeout = impl->getField(env->exec, "timeout")->getInt32();
    uint64_t startTime = getNanoTime() / 1000000;

    while(!env->exec->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getNanoTime() / 1000000 - startTime)) < timeout)) {
        int32_t n;
        SocketError err = Socket_Receive(sock, &b->getData()[off], len, &n);
        if(err == SOCKET_OK) return n;
        else if(err == SOCKET_CLOSED) return -1;
        else if(err == SOCKET_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Read error");
            return -1;
        }
    }
    if(!env->exec->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Read timed out");
    return -1;
}
