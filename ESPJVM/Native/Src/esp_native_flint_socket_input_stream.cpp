
#include <sys/socket.h>
#include <arpa/inet.h>
#include "flint_java_object.h"
#include "esp_native_common.h"
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_impl.h"
#include "esp_native_flint_socket_input_stream.h"

using namespace FlintAPI::System;

jint NativeFlintSocketInputStream_SocketRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj);
    if(sock < 0) return -1;

    if(!CheckArrayIndexSize(env, b, off, len)) return -1;

    jobject impl = obj->getField(env->exec, "impl")->getObj();
    int32_t timeout = impl->getField(env->exec, "timeout")->getInt32();
    uint64_t startTime = getNanoTime() / 1000000;

    while(!env->exec->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getNanoTime() / 1000000 - startTime)) < timeout)) {
        int32_t n = recv(sock, &b->getData()[off], len, 0);
        if(n > 0) return n;
        else if(n == 0) return -1;
        else if(errno == EINTR || errno == EAGAIN) continue;
        else {
            env->throwNew(env->findClass("java/io/IOException"), "Socket write error");
            return -1;
        }
    }
    if(!env->exec->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Read timed out");
    return -1;
}
