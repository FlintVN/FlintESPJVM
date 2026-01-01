
#ifndef __ESP_SOCKET_H
#define __ESP_SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include "flint_native.h"

typedef enum {
    SOCKET_OK,
    SOCKET_CLOSED,
    SOCKET_TIMEOUT,
    SOCKET_PENDING,
    SOCKET_INPROGRESS,
    SOCKET_ERR,
} SocketError;

int32_t Socket_Create(bool stream);
SocketError Socket_Connect(int32_t sock, struct sockaddr_in6 *addr);
SocketError Socket_GetConnectionStatus(int32_t sock, bool *isConnected);
SocketError Socket_Bind(int32_t sock, struct sockaddr_in6 *addr);
SocketError Socket_Listen(int32_t sock, int32_t count);
SocketError Socket_Accept(int32_t server, struct sockaddr_in6 *addr, int32_t *client);
SocketError Socket_Send(int32_t sock, int8_t *data, int32_t len, int32_t *sent);
SocketError Socket_SendTo(int32_t sock, struct sockaddr_in6 *addr, int8_t *data, int32_t len, int32_t *sent);
SocketError Socket_Receive(int32_t sock, int8_t *buf, int32_t len, int32_t *read);
SocketError Socket_ReceiveFrom(int32_t sock, struct sockaddr_in6 *addr, int8_t *buf, int32_t len, int32_t *read);
int32_t Socket_Available(int32_t sock);
SocketError Socket_SetOption(int32_t sock, int32_t level, int32_t optname, const void *optval, int32_t optlen);
SocketError Socket_GetOption(int32_t sock, int32_t level, int32_t optname, void *optval, socklen_t *optlen);
void Socket_Close(int32_t sock);

void Socket_Reset(void);

#endif /* __ESP_SOCKET_H */
