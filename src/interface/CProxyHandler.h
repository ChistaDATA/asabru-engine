#ifndef PROXY_HANDLER_H
#define PROXY_HANDLER_H

#include <string>
#include "CommonTypes.h"
#include "../socket/CServerSocket.h"
#include "../socket/SocketClient.h"

class CProxyHandler
{
public:
    CProxyHandler() {}

    virtual std::string HandleUpstreamData(void *buffer, int buffer_length, EXECUTION_CONTEXT *exec_context) = 0;

    virtual std::string HandleDownStreamData(void *buffer, int buffer_length, EXECUTION_CONTEXT *exec_context) = 0;

    virtual ~CProxyHandler() {}
};

#endif