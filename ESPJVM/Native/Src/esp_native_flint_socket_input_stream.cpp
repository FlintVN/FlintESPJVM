
#include <sys/socket.h>
#include <arpa/inet.h>
#include "esp_native_common.h"
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_impl.h"
#include "esp_native_flint_socket_input_stream.h"

jint NativeFlintSocketInputStream_SocketRead(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj);
    if(sock < 0) return -1;

    if(!CheckArrayIndexSize(env, b, off, len)) return -1;

    while(!env->exec->hasTerminateRequest()) {
        int32_t n = recv(sock, &b->getData()[off], len, 0);
        if(n > 0) return n;
        else if(n == 0) return -1;
        else if(errno == EINTR || errno == EAGAIN) continue;
        else {
            env->throwNew(env->findClass("java/io/IOException"), "socket write error");
            return -1;
        }
    }
    return -1;
}
