
#include <sys/socket.h>
#include <arpa/inet.h>
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_impl.h"

#define IPV4            1
#define IPV6            2

using namespace FlintAPI::System;

static uint32_t sockListLength = 0;
static int32_t *sockList = NULL;

static int32_t SockListAdd(FNIEnv *env, int32_t sock) {
    Flint::lock();
    for(uint32_t i = 0; i < sockListLength; i++) {
        if(sockList[i] < 0) {
            sockList[i] = sock;
            Flint::unlock();
            return i;
        }
    }
    int32_t *newList = (int32_t *)Flint::realloc(env->exec, sockList, sockListLength + 10);
    if(newList == NULL) {
        Flint::unlock();
        return -1;
    }
    sockList = newList;
    sockList[sockListLength] = sock;
    sockListLength += 10;
    Flint::unlock();
    return sockListLength - 10;
}

static void SockListRemove(int32_t sock) {
    Flint::lock();
    for(uint32_t i = 0; i < sockListLength; i++) {
        if(sockList[i] == sock) {
            sockList[i] = -1;
            return;
        }
    }
    Flint::unlock();
}

static void SockClose(int32_t sock) {
    close(sock);
    SockListRemove(sock);
}

void NativeFlintSocketImpl_Reset(void) {
    if(sockListLength > 0) {
        for(uint32_t i = 0; i < sockListLength; i++) {
            if(sockList[i] >= 0)
                close(sockList[i]);
        }
        Flint::free(sockList);
    }
    sockListLength = 0;
    sockList = NULL;
}

static bool GetIPv6(InetAddress *inetAddr, int32_t port, struct sockaddr_in6 *ipv6) {
    int32_t family = inetAddr->getFamily();
    if(family != IPV4 && family != IPV6)
        return false;

    memset(ipv6, 0, sizeof(*ipv6));
    ipv6->sin6_family = AF_INET6;
    ipv6->sin6_port = htons(port);
    if(family == IPV4) {
        Inet4Address *inet4Addr = (Inet4Address *)inetAddr;
        ipv6->sin6_addr.un.u8_addr[10] = 0xFF;
        ipv6->sin6_addr.un.u8_addr[11] = 0xFF;
        ipv6->sin6_addr.un.u8_addr[12] = inet4Addr->getAddress() >> 24;
        ipv6->sin6_addr.un.u8_addr[13] = inet4Addr->getAddress() >> 16;
        ipv6->sin6_addr.un.u8_addr[14] = inet4Addr->getAddress() >> 8;
        ipv6->sin6_addr.un.u8_addr[15] = inet4Addr->getAddress() >> 0;
    }
    else {
        Inet6Address *inet6Addr = (Inet6Address *)inetAddr;
        memcpy(ipv6->sin6_addr.un.u8_addr, inet6Addr->getAddress()->getData(), 16);
        if(inet6Addr->getScopeIdSet())
            ipv6->sin6_scope_id = inet6Addr->getScopeId();
    }

    return true;
}

int32_t NativeFlintSocketImpl_GetSock(FNIEnv *env, jobject socketObj, bool throwable) {
    jobject fdObj = socketObj->getField(env->exec, "fd")->getObj();
    if(fdObj == NULL) {
        if(throwable)
            env->throwNew(env->findClass("java/io/IOException"), "Socket has not been created");
        return -1;
    }
    int32_t sock = fdObj->getField(env->exec, "fd")->getInt32();
    if(sock < 0) {
        if(throwable)
            env->throwNew(env->findClass("java/io/IOException"), "Socket has not been created");
    }
    return sock;
}

static bool IsIPv4MappedAddress(uint8_t *addr) {
    if(
        (addr[0] == 0x00) && (addr[1] == 0x00) &&
        (addr[2] == 0x00) && (addr[3] == 0x00) &&
        (addr[4] == 0x00) && (addr[5] == 0x00) &&
        (addr[6] == 0x00) && (addr[7] == 0x00) &&
        (addr[8] == 0x00) && (addr[9] == 0x00) &&
        (addr[10] == 0xFF) && (addr[11] == 0xFF)
    ) {
        return true;
    }
    return false;
}

jvoid NativeFlintSocketImpl_SocketCreate(FNIEnv *env, jobject obj) {
    jint sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_IP);
    if(sock < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Create socket error");
        return;
    }
    SockListAdd(env, sock);

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    jobject fdObj = obj->getField(env->exec, "fd")->getObj();
    fdObj->getField(env->exec, "fd")->setInt32(sock);
}

jvoid NativeFlintSocketImpl_SocketConnect(FNIEnv *env, jobject obj, jobject address, jint port) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    InetAddress *inetAddr = (InetAddress *)address;
    struct sockaddr_in6 addr;
    if(!GetIPv6(inetAddr, port, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }

    int32_t ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    if(ret == -1) {
        if(errno == EINPROGRESS) {
            struct pollfd pfd = {};
            pfd.fd = sock;
            pfd.events = POLLOUT;
            while(!env->exec->hasTerminateRequest()) {
                ret = poll(&pfd, 1, 100);
                if(ret > 0) {
                    if(pfd.revents & (POLLOUT | POLLIN)) {
                        int32_t err;
                        socklen_t len = sizeof(err);
                        getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
                        if(err == 0) return;
                        break;
                    }
                    else if(pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
                        break;
                }
                else if(ret < 0 && errno != EINTR)
                    break;
            }
        }
        env->throwNew(env->findClass("java/io/IOException"), "Connect error");
    }
}

jvoid NativeFlintSocketImpl_SocketBind(FNIEnv *env, jobject obj, jobject address, jint port) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    InetAddress *inetAddr = (InetAddress *)address;
    struct sockaddr_in6 addr;
    if(!GetIPv6(inetAddr, port, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }
    int32_t ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if(ret != 0)
        env->throwNew(env->findClass("java/io/IOException"), "Bind error with error code %d", ret);
}

jvoid NativeFlintSocketImpl_SocketListen(FNIEnv *env, jobject obj, jint count) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    int32_t ret = listen(sock, count);
    if(ret != 0)
        env->throwNew(env->findClass("java/io/IOException"), "Listen error with error code %d", ret);
}

jvoid NativeFlintSocketImpl_SocketAccept(FNIEnv *env, jobject obj, jobject s) {
    int32_t listenSock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(listenSock < 0) return;

    struct sockaddr_in6 addr;
    socklen_t addrLen = sizeof(addr);

    int32_t timeout = obj->getField(env->exec, "timeout")->getInt32();
    uint64_t startTime = getNanoTime() / 1000000;

    while(!env->exec->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getNanoTime() / 1000000 - startTime)) < timeout)) {
        int32_t sock = accept(listenSock, (struct sockaddr *)&addr, &addrLen);
        if(sock >= 0) {
            SockListAdd(env, sock);
            if(IsIPv4MappedAddress(addr.sin6_addr.un.u8_addr)) {
                int32_t ipv4 = addr.sin6_addr.un.u8_addr[12] << 24;
                ipv4 |= addr.sin6_addr.un.u8_addr[13] << 16;
                ipv4 |= addr.sin6_addr.un.u8_addr[14] << 8;
                ipv4 |= addr.sin6_addr.un.u8_addr[15] << 0;

                Inet4Address *inetAddr = (Inet4Address *)env->newObject(env->findClass("java/net/Inet4Address"));
                if(inetAddr == NULL) return;

                inetAddr->setHostName(NULL);
                inetAddr->setFamily(IPV4);
                inetAddr->setAddress(ipv4);

                s->getField(env->exec, "address")->setObj(inetAddr);
                inetAddr->clearProtected();
            }
            else {
                Inet6Address *inetAddr = (Inet6Address *)env->newObject(env->findClass("java/net/Inet6Address"));
                jbyteArray byteArr = env->newByteArray(16);
                if(byteArr == NULL || inetAddr == NULL) {
                    if(inetAddr != NULL) env->freeObject(inetAddr);
                    if(byteArr != NULL) env->freeObject(byteArr);
                    return;
                }
                memcpy(byteArr->getData(), addr.sin6_addr.un.u8_addr, 16);

                inetAddr->setHostName(NULL);
                inetAddr->setFamily(IPV6);
                inetAddr->setAddress(byteArr);
                inetAddr->setScopeId(addr.sin6_scope_id);
                if(addr.sin6_scope_id > 0)
                    inetAddr->setScopeIdSet(true);
                s->getField(env->exec, "address")->setObj(inetAddr);
                byteArr->clearProtected();
                inetAddr->clearProtected();
            }
            s->getField(env->exec, "fd")->getObj()->getField(env->exec, "fd")->setInt32(sock);
            s->getField(env->exec, "port")->setInt32(obj->getField(env->exec, "port")->getInt32());
            s->getField(env->exec, "localport")->setInt32(obj->getField(env->exec, "localport")->getInt32());
            return;
        }
        else if(errno == EINTR || errno == EAGAIN)
            continue;
        else {
            env->throwNew(env->findClass("java/io/IOException"), "Accept error");
            return;
        }
    }
    if(!env->exec->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Accept timed out");
}

jint NativeFlintSocketImpl_SocketAvailable(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return -1;

    int32_t bytes = 0;
    if(ioctl(sock, FIONREAD, &bytes) < 0)
        return -1;
    return bytes;
}

jvoid NativeFlintSocketImpl_SocketClose(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, false);
    if(sock < 0) return;
    SockClose(sock);
}

jvoid NativeFlintSocketImpl_InitProto(FNIEnv *env) {
    /* Do nothing */
}

jvoid NativeFlintSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint cmd, jbool on, jobject value) {
    // TODO
}

jint NativeFlintSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt) {
    // TODO
    return 0;
}
