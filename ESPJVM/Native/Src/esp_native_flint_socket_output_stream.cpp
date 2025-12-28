
#include <sys/socket.h>
#include <arpa/inet.h>
#include "esp_native_common.h"
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_impl.h"
#include "esp_native_flint_socket_output_stream.h"

jvoid NativeFlintSocketOutputStream_SocketWrite(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    if(!CheckArrayIndexSize(env, b, off, len)) return;

    while(len > 0 && !env->exec->hasTerminateRequest()) {
        ssize_t size = send(sock, &b->getData()[off], len, 0);
        if(size == 0) return;
        if(size < 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            env->throwNew(env->findClass("java/io/IOException"), "Socket write error");
            return;
        }
        len -= size;
        off += size;
    }
}
