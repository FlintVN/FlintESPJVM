
#include "esp_socket.h"
#include "esp_native_common.h"
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_impl.h"
#include "esp_native_flint_socket_output_stream.h"

jvoid NativeFlintSocketOutputStream_SocketWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    if(!CheckArrayIndexSize(env, b, off, len)) return;

    while(len > 0 && !env->exec->hasTerminateRequest()) {
        int32_t sent;
        SocketError err = Socket_Send(sock, &b->getData()[off], len, &sent);
        if(err == SOCKET_OK) {
            len -= sent;
            off += sent;
        }
        else if(err == SOCKET_CLOSED)
            return;
        else if(err == SOCKET_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Socket write error");
            return;
        }
    }
}
