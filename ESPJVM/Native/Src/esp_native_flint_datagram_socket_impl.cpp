
#include "esp_socket.h"
#include "esp_native_common.h"
#include "esp_flint_java_inet_address.h"
#include "esp_native_flint_socket_impl.h"
#include "esp_native_flint_datagram_socket_impl.h"

#define IPV4                    1
#define IPV6                    2

#define NATIVE_TCP_NODELAY      0x0001
#define NATIVE_SO_BINDADDR      0x000F
#define NATIVE_SO_REUSEADDR     0x04
#define NATIVE_IP_MULTICAST_IF  0x10
#define NATIVE_SO_LINGER        0x0080
#define NATIVE_SO_TIMEOUT       0x1006

using namespace FlintAPI::System;

extern InetAddress *NativeFlintSocketImpl_CreateInetAddress(FNIEnv *env, struct sockaddr_in6 *addr);

class DatagramPacket : public JObject {
public:
    jbyteArray getBuf() { return (jbyteArray)getFieldByIndex(0)->getObj(); }
    jint getLength() { return getFieldByIndex(1)->getInt32(); }
    InetAddress *getAddress() { return (InetAddress *)getFieldByIndex(2)->getObj(); };
    jint getPort() { return getFieldByIndex(3)->getInt32(); }

    void setBuf(jbyteArray val) { getFieldByIndex(0)->setObj(val); }
    void setLength(jint val) { getFieldByIndex(1)->setInt32(val); }
    void setAddress(InetAddress *val) { getFieldByIndex(2)->setObj(val); };
    void setPort(jint val) { getFieldByIndex(3)->setInt32(val); }
};

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

static int32_t NativeFlintDatagramSocketImpl_GetSock(FNIEnv *env, jobject socketObj, bool throwable) {
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

jvoid NativeFlintDatagramSocketImpl_Bind(FNIEnv *env, jobject obj, jint lport, jobject laddr) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    InetAddress *inetAddr = (InetAddress *)laddr;
    struct sockaddr_in6 addr;
    if(!GetIPv6(inetAddr, lport, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }
    if(Socket_Bind(sock, &addr) != SOCKET_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Bind error");
    else
        obj->getField(env->exec, "localPort")->setInt32(lport);
}

jvoid NativeFlintDatagramSocketImpl_Send(FNIEnv *env, jobject obj, jobject p) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    if(p == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return;
    }

    jbyteArray b = ((DatagramPacket *)p)->getBuf();
    jint len = ((DatagramPacket *)p)->getLength();
    InetAddress *inetAddr = ((DatagramPacket *)p)->getAddress();
    jint port = ((DatagramPacket *)p)->getPort();

    jint off = 0;
    struct sockaddr_in6 addr;
    if(inetAddr == NULL) {
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "Address not set");
        return;
    }
    else if(!GetIPv6(inetAddr, port, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }

    if(!CheckArrayIndexSize(env, b, 0, len)) return;

    while(len > 0 && !env->exec->hasTerminateRequest()) {
        int32_t sent;
        SocketError err = Socket_SendTo(sock, &addr, &b->getData()[off], len, &sent);
        if(err == SOCKET_OK) {
            len -= sent;
            off += sent;
        }
        else if(err == SOCKET_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Write error");
            return;
        }
    }
}

jint NativeFlintDatagramSocketImpl_Peek(FNIEnv *env, jobject obj, jobject i) {
    // TODO
    return 0;
}

jvoid NativeFlintDatagramSocketImpl_Receive(FNIEnv *env, jobject obj, jobject p) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    if(p == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return;
    }

    jint off = 0;
    DatagramPacket *packet = (DatagramPacket *)p;
    jbyteArray b = packet->getBuf();
    jint len = packet->getLength();

    if(!CheckArrayIndexSize(env, b, off, len)) return;

    int32_t timeout = obj->getField(env->exec, "timeout")->getInt32();
    uint64_t startTime = getTimeMillis();

    while(!env->exec->hasTerminateRequest() && (timeout <= 0 || ((uint64_t)(getTimeMillis() - startTime)) < timeout)) {
        int32_t n;
        struct sockaddr_in6 addr;
        SocketError err = Socket_ReceiveFrom(sock, &addr, b->getData(), len, &n);
        if(err == SOCKET_OK) {
            InetAddress *inetAddr = NativeFlintSocketImpl_CreateInetAddress(env, &addr);
            if(inetAddr == NULL) return;
            packet->setAddress(inetAddr);
            inetAddr->clearProtected();
            if(inetAddr->getFamily() == IPV6)
                ((Inet6Address *)inetAddr)->getAddress()->clearProtected();
            packet->setPort((int32_t)ntohs(addr.sin6_port));
            packet->setLength(n);
            return;
        }
        else if(err == SOCKET_ERR) {
            env->throwNew(env->findClass("java/io/IOException"), "Read error");
            return;
        }
    }
    if(!env->exec->hasTerminateRequest())
        env->throwNew(env->findClass("java/net/SocketTimeoutException"), "Read timed out");
}

jvoid NativeFlintDatagramSocketImpl_SetTTL(FNIEnv *env, jobject obj, jbyte ttl) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    int32_t hopLimit = ttl;
    if(Socket_SetOption(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &hopLimit, sizeof(hopLimit)) != SOCKET_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Set TTL error");
}

jbyte NativeFlintDatagramSocketImpl_GetTTL(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return 0;

    int32_t hopLimit;
    socklen_t optlen = sizeof(hopLimit);
    if(Socket_GetOption(sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &hopLimit, &optlen) != SOCKET_OK)
        env->throwNew(env->findClass("java/io/IOException"), "Get TTL error");
    return hopLimit;
}

jvoid NativeFlintDatagramSocketImpl_Join(FNIEnv *env, jobject obj, jobject inetaddr) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    InetAddress *inetObj = (InetAddress *)inetaddr;
    struct sockaddr_in6 addr;
    if(!GetIPv6(inetObj, 0, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }

    struct ipv6_mreq mreq6 = {};
    mreq6.ipv6mr_multiaddr = addr.sin6_addr;
    if(inetObj->getFamily() == IPV6 && ((Inet6Address *)inetObj)->getScopeIdSet())
        mreq6.ipv6mr_interface = ((Inet6Address *)inetObj)->getScopeId();
    if(Socket_SetOption(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &mreq6, sizeof(mreq6)) != SOCKET_OK)
        env->throwNew(env->findClass("java/net/IOException"), "Join error");
}

jvoid NativeFlintDatagramSocketImpl_Leave(FNIEnv *env, jobject obj, jobject inetaddr) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    InetAddress *inetObj = (InetAddress *)inetaddr;
    struct sockaddr_in6 addr;
    if(!GetIPv6(inetObj, 0, &addr)) {
        env->throwNew(env->findClass("java/io/IOException"), "Invalid address");
        return;
    }

    struct ipv6_mreq mreq6 = {};
    mreq6.ipv6mr_multiaddr = addr.sin6_addr;
    if(inetObj->getFamily() == IPV6 && ((Inet6Address *)inetObj)->getScopeIdSet())
        mreq6.ipv6mr_interface = ((Inet6Address *)inetObj)->getScopeId();

    if(Socket_SetOption(sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &mreq6, sizeof(mreq6)) != SOCKET_OK)
        env->throwNew(env->findClass("java/net/IOException"), "Leave error");
}

jvoid NativeFlintDatagramSocketImpl_DatagramSocketCreate(FNIEnv *env, jobject obj) {
    int32_t sock = Socket_Create(false);
    if(sock < 0) {
        env->throwNew(env->findClass("java/io/IOException"), "Create DatagramSocket error");
        return;
    }
    jobject fdObj = obj->getField(env->exec, "fd")->getObj();
    fdObj->getField(env->exec, "fd")->setInt32(sock);
}

jvoid NativeFlintDatagramSocketImpl_DatagramSocketClose(FNIEnv *env, jobject obj) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;
    Socket_Close(sock);
}

jvoid NativeFlintDatagramSocketImpl_SocketSetOption(FNIEnv *env, jobject obj, jint opt, jobject val) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return;

    switch(opt) {
        case NATIVE_SO_REUSEADDR: {
            if(val == NULL) {
                env->throwNew(env->findClass("java/lang/NullPointerException"));
                return;
            }
            int32_t optval = val->getFieldByIndex(0)->getInt32();
            if(Socket_SetOption(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) != SOCKET_OK)
                env->throwNew(env->findClass("java/io/IOException"), "Set SO_REUSEADDR error");
            break;
        }
        default:
            env->throwNew(env->findClass("java/io/IOException"), "Unsupported socket option");
            break;
    }
}

jobject NativeFlintDatagramSocketImpl_SocketGetOption(FNIEnv *env, jobject obj, jint opt) {
    int32_t sock = NativeFlintDatagramSocketImpl_GetSock(env, obj, true);
    if(sock < 0) return NULL;

    switch(opt) {
        case NATIVE_SO_REUSEADDR: {
            int32_t val = 0;
            socklen_t optlen = sizeof(val);
            if(Socket_GetOption(sock, SOL_SOCKET, SO_REUSEADDR, &val, &optlen) != SOCKET_OK) {
                env->throwNew(env->findClass("java/io/IOException"), "Get SO_REUSEADDR error");
                return NULL;
            }
            jobject ret = env->newObject(env->findClass("java/lang/Integer"));
            if(ret == NULL) return NULL;
            ret->getFieldByIndex(0)->setInt32(val);
            return ret;
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
