#ifndef PROXY_HANDLER_H
#define PROXY_HANDLER_H

#include "../socket/ServerSocket.h"
#include "../socket/SocketClient.h"

class CProxyHandler
{
public:
    CProxyHandler() {}

    virtual void * HandleUpstreamData(void *buffer, int buffer_length, SocketClient *target) = 0;

    virtual void * HandleDownStreamData(void *buffer, int buffer_length, Socket *client) = 0;

    virtual ~CProxyHandler() {}
};

#endif