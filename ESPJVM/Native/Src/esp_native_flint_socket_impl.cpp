
#include "esp_socket.h"
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_impl.h"

#define IPV4                    1
#define IPV6                    2

#define NATIVE_TCP_NODELAY      0x0001
#define NATIVE_SO_BINDADDR      0x000F
#define NATIVE_SO_REUSEADDR     0x04
#define NATIVE_IP_MULTICAST_IF  0x10
#define NATIVE_SO_LINGER        0x0080
#define NATIVE_SO_TIMEOUT       0x1006

using namespace FlintAPI::System;

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

InetAddress *NativeFlintSocketImpl_CreateInetAddress(FNIEnv *env, struct sockaddr_in6 *addr) {
    if(IsIPv4MappedAddress(addr->sin6_addr.un.u8_addr)) {
        int32_t ipv4 = addr->sin6_addr.un.u8_addr[12] << 24;
        ipv4 |= addr->sin6_addr.un.u8_addr[13] << 16;
        ipv4 |= addr->sin6_addr.un.u8_addr[14] << 8;
        ipv4 |= addr->sin6_addr.un.u8_addr[15] << 0;

        Inet4Address *inetAddr = (Inet4Address *)env->newObject(env->findClass("java/net/Inet4Address"));
        if(inetAddr == NULL) return NULL;

        inetAddr->setHostName(NULL);
        inetAddr->setFamily(IPV4);
        inetAddr->setAddress(ipv4);

        return inetAddr;
    }
    else {
        Inet6Address *inetAddr = (Inet6Address *)env->newObject(env->findClass("java/net/Inet6Address"));
        jbyteArray byteArr = env->newByteArray(16);
        if(byteArr == NULL || inetAddr == NULL) {
            if(inetAddr != NULL) env->freeObject(inetAddr);
            if(byteArr != NULL) env->freeObject(byteArr);
            return NULL;
        }
        memcpy(byteArr->getData(), addr->sin6_addr.un.u8_addr, 16);

        inetAddr->setHostName(NULL);
        inetAddr->setFamily(IPV6);
        inetAddr->setAddress(byteArr);
        inetAddr->setScopeId(addr->sin6_scope_id);
        if(addr->sin6_scope_id > 0)
            inetAddr->setScopeIdSet(true);
        return inetAddr;
    }
}

jvoid NativeFlintSocketImpl_SocketCreate(FNIEnv *env, jobject obj) {
    int32_t sock = Socket_Create(true);
    if(sock < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Create socket error");
        return;
    }
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

    SocketError err = Socket_Connect(sock, &addr);
    if(err == SOCKET_OK) return;
    else if(err == SOCKET_INPROGRESS) {
        while(!env->exec->hasTerminateRequest()) {
            bool isConnected;
            if(Socket_GetConnectionStatus(sock, &isConnected) != SOCKET_OK)
                break;
            if(isConnected == true)
                return;
        }
    }
    env->throwNew(env->findClass("java/io/IOException"), "Connect error");
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
    if(Socket_Bind(sock, &addr) != SOCKET_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Bind error");
}

jvoid NativeFlintSocketImpl_SocketListen(FNIEnv *env, jobject obj, jint count) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    if(Socket_Listen(sock, count) != SOCKET_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Listen error");
}

jvoid NativeFlintSocketImpl_SocketAccept(FNIEnv *env, jobject obj, jobject s) {
    int32_t listenSock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(listenSock < 0) return;

    struct sockaddr_in6 addr;

    int32_t timeout = obj->getField(env->exec, "timeout")->getInt32();
    uint64_t startTime = getTimeMillis();

    while(!env->exec->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getTimeMillis() - startTime)) < timeout)) {
        int32_t client;
        SocketError err = Socket_Accept(listenSock, &addr, &client);
        if(err == SOCKET_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Accept error");
            return;
        }
        else if(err == SOCKET_OK) {
            InetAddress *inetAddr = NativeFlintSocketImpl_CreateInetAddress(env, &addr);
            if(inetAddr == NULL) return;
            s->getField(env->exec, "address")->setObj(inetAddr);
            inetAddr->clearProtected();
            if(inetAddr->getFamily() == IPV6)
                ((Inet6Address *)inetAddr)->getAddress()->clearProtected();
            s->getField(env->exec, "fd")->getObj()->getField(env->exec, "fd")->setInt32(client);
            s->getField(env->exec, "port")->setInt32(obj->getField(env->exec, "port")->getInt32());
            s->getField(env->exec, "localport")->setInt32(obj->getField(env->exec, "localport")->getInt32());
            return;
        }
    }
    if(!env->exec->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Accept timed out");
}

jint NativeFlintSocketImpl_SocketAvailable(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return -1;
    return Socket_Available(sock);
}

jvoid NativeFlintSocketImpl_SocketClose(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, false);
    if(sock < 0) return;
    Socket_Close(sock);
}

jvoid NativeFlintSocketImpl_InitProto(FNIEnv *env) {
    /* Do nothing */
}

jvoid NativeFlintSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint cmd, jbool on, jobject value) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    switch(cmd) {
        case NATIVE_TCP_NODELAY: {
            int32_t opt = on ? 1 : 0;
            if(Socket_SetOption(sock, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) != SOCKET_OK)
                env->throwNew(env->findClass("java/io/IOException"), "Set TCP_NODELAY error");
            break;
        }
        case NATIVE_SO_LINGER: {
            struct linger ling = {};
            ling.l_onoff = on ? 1 : 0;
            ling.l_linger = (value != NULL) ? value->getFieldByIndex(0)->getInt32() : 0;
            if(Socket_SetOption(sock, SOL_SOCKET, SO_LINGER, &ling, sizeof(ling)) != SOCKET_OK)
                env->throwNew(env->findClass("java/io/IOException"), "Set SO_LINGER error");
            break;
        }
        default:
            env->throwNew(env->findClass("java/io/IOException"), "Unsupported socket option");
            break;
    }
}

jobject NativeFlintSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt) {
    int32_t sock = NativeFlintSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return NULL;

    switch(opt) {
        case NATIVE_SO_TIMEOUT: {
            jobject ret = env->newObject(env->findClass("java/lang/Integer"));
            if(ret == NULL) return NULL;
            ret->getFieldByIndex(0)->setInt32(obj->getField(env->exec, "timeout")->getInt32());
            return ret;
        }
        case NATIVE_TCP_NODELAY: {
            int32_t val = 0;
            socklen_t optlen = sizeof(val);
            if(Socket_GetOption(sock, IPPROTO_TCP, TCP_NODELAY, &val, &optlen) != SOCKET_OK) {
                env->throwNew(env->findClass("java/io/IOException"), "Get TCP_NODELAY error");
                return NULL;
            }
            jobject ret = env->newObject(env->findClass("java/lang/Integer"));
            if(ret == NULL) return NULL;
            ret->getFieldByIndex(0)->setInt32(val);
            return ret;
        }
        case NATIVE_SO_LINGER: {
            struct linger ling = {};
            socklen_t optlen = sizeof(ling);
            if(Socket_GetOption(sock, SOL_SOCKET, SO_LINGER, &ling, &optlen) != SOCKET_OK) {
                env->throwNew(env->findClass("java/io/IOException"), "Get SO_LINGER error");
                return NULL;
            }
            if(ling.l_onoff == 0) {
                jobject ret = env->newObject(env->findClass("java/lang/Boolean"));
                if(ret == NULL) return NULL;
                ret->getFieldByIndex(0)->setInt32(0);
                return ret;
            }
            else {
                jobject ret = env->newObject(env->findClass("java/lang/Integer"));
                if(ret == NULL) return NULL;
                ret->getFieldByIndex(0)->setInt32(ling.l_linger);
                return ret;
            }
        }
        case NATIVE_SO_BINDADDR: {
            struct sockaddr_in6 addr;
            socklen_t addrlen = sizeof(addr);
            if(getsockname(sock, (struct sockaddr *)&addr, &addrlen) != 0) {
                env->throwNew(env->findClass("java/io/IOException"), "Get SO_BINDADDR error");
                return NULL;
            }
            return NativeFlintSocketImpl_CreateInetAddress(env, &addr);
        }
        default:
            env->throwNew(env->findClass("java/io/IOException"), "Unsupported socket option");
            return NULL;
    }
}
