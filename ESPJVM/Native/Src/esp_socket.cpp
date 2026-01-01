
#include "flint.h"
#include "esp_socket.h"

static uint32_t sockListLength = 0;
static int32_t *sockList = NULL;

static int32_t SockList_Add(int32_t sock) {
    Flint::lock();
    for(uint32_t i = 0; i < sockListLength; i++) {
        if(sockList[i] < 0) {
            sockList[i] = sock;
            Flint::unlock();
            return i;
        }
    }
    int32_t *newList = (int32_t *)Flint::realloc(NULL, sockList, sockListLength + 10);
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

static void SockList_Remove(int32_t sock) {
    Flint::lock();
    for(uint32_t i = 0; i < sockListLength; i++) {
        if(sockList[i] == sock) {
            sockList[i] = -1;
            return;
        }
    }
    Flint::unlock();
}

int32_t Socket_Create(bool stream) {
    jint sock = socket(AF_INET6, stream ? SOCK_STREAM : SOCK_DGRAM, IPPROTO_IP);
    if(sock < 0)
        return -1;
    if(SockList_Add(sock) < 0) {
        Socket_Close(sock);
        return -1;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    return sock;
}

SocketError Socket_Connect(int32_t sock, struct sockaddr_in6 *addr) {
    int32_t ret = connect(sock, (struct sockaddr *)addr, sizeof(sockaddr_in6));
    if(ret == -1)
        return (errno == EINPROGRESS) ? SOCKET_INPROGRESS : SOCKET_ERR;
    return SOCKET_OK;
}

SocketError Socket_GetConnectionStatus(int32_t sock, bool *isConnected) {
    struct pollfd pfd = {};
    pfd.fd = sock;
    pfd.events = POLLOUT;

    int32_t ret = poll(&pfd, 1, 0);
    if(ret > 0) {
        if(pfd.revents & (POLLOUT | POLLIN)) {
            int32_t err;
            socklen_t len = sizeof(err);
            getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
            if(err == 0) {
                *isConnected = true;
                return SOCKET_OK;
            }
            return SOCKET_ERR;
        }
        else if(pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
            return SOCKET_ERR;
        *isConnected = false;
        return SOCKET_OK;
    }
    else if(ret < 0 && errno != EINTR)
        return SOCKET_ERR;
    *isConnected = false;
    return SOCKET_OK;
}

SocketError Socket_Bind(int32_t sock, struct sockaddr_in6 *addr) {
    return bind(sock, (struct sockaddr *)addr, sizeof(sockaddr_in6)) == 0 ? SOCKET_OK : SOCKET_ERR;
}

SocketError Socket_Listen(int32_t sock, int32_t count) {
    return listen(sock, count) == 0 ? SOCKET_OK : SOCKET_ERR;
}

SocketError Socket_Accept(int32_t server, struct sockaddr_in6 *addr, int32_t *client) {
    socklen_t addrLen = sizeof(sockaddr_in6);
    *client = accept(server, (struct sockaddr *)addr, &addrLen);
    if(*client >= 0) {
        SockList_Add(*client);
        return SOCKET_OK;
    }
    else if(errno == EINTR || errno == EAGAIN)
        return SOCKET_TIMEOUT;
    return SOCKET_ERR;
}

SocketError Socket_Send(int32_t sock, int8_t *data, int32_t len, int32_t *sent) {
    ssize_t n = send(sock, data, len, 0);
    if(n == 0) return SOCKET_CLOSED;
    if(n < 0) {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return SOCKET_PENDING;
        return SOCKET_ERR;
    }
    *sent = n;
    return SOCKET_OK;
}

SocketError Socket_SendTo(int32_t sock, struct sockaddr_in6 *addr, int8_t *data, int32_t len, int32_t *sent) {
    ssize_t n = sendto(sock, data, len, 0, (struct sockaddr *)addr, sizeof(sockaddr_in6));
    if(n > 0) {
        *sent = n;
        return SOCKET_OK;
    }
    else if(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
        return SOCKET_PENDING;
    return SOCKET_ERR;
}

SocketError Socket_Receive(int32_t sock, int8_t *buf, int32_t len, int32_t *read) {
    int32_t n = recv(sock, buf, len, 0);
    if(n > 0) {
        *read = n;
        return SOCKET_OK;
    }
    else if(n == 0)
        return SOCKET_CLOSED;
    else if(errno == EINTR || errno == EAGAIN)
        return SOCKET_TIMEOUT;
    return SOCKET_ERR;
}

SocketError Socket_ReceiveFrom(int32_t sock, struct sockaddr_in6 *addr, int8_t *buf, int32_t len, int32_t *read) {
    socklen_t addrlen = sizeof(sockaddr_in6);
    int32_t n = recvfrom(sock, buf, len, 0, (struct sockaddr *)addr, &addrlen);
    if(n > 0) {
        *read = n;
        return SOCKET_OK;
    }
    else if(n < 0 && (errno == EINTR || errno == EAGAIN))
        return SOCKET_TIMEOUT;
    return SOCKET_ERR;
}

int32_t Socket_Available(int32_t sock) {
    int32_t bytes = 0;
    if(ioctl(sock, FIONREAD, &bytes) < 0)
        return -1;
    return bytes;
}

SocketError Socket_SetOption(int32_t sock, int32_t level, int32_t optname, const void *optval, int32_t optlen) {
    return setsockopt(sock, level, optname, optval, optlen) == 0 ? SOCKET_OK : SOCKET_ERR;
}

SocketError Socket_GetOption(int32_t sock, int32_t level, int32_t optname, void *optval, socklen_t *optlen) {
    return getsockopt(sock, level, optname, optval, optlen) == 0 ? SOCKET_OK : SOCKET_ERR;
}

void Socket_Close(int32_t sock) {
    close(sock);
    SockList_Remove(sock);
}

void Socket_Reset(void) {
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
