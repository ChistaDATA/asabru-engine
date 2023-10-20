#ifndef PROXY_HANDLER_H
#define PROXY_HANDLER_H

#include <string>
#include "CommonTypes.h"
#include "BaseHandler.h"
#include "../socket/CServerSocket.h"
#include "../socket/CClientSocket.h"

class CProxyHandler : BaseHandler
{
public:
    CProxyHandler() {}

    virtual std::string HandleUpstreamData(void *buffer, ssize_t buffer_length, EXECUTION_CONTEXT *exec_context) = 0;

    virtual std::string HandleDownStreamData(void *buffer, ssize_t buffer_length, EXECUTION_CONTEXT *exec_context) = 0;

    virtual ~CProxyHandler() {}
};

#endif