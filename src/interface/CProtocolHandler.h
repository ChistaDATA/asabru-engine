#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include "../socket/CServerSocket.h"
#include "BaseHandler.h"
#include "CommonTypes.h"

class CProtocolHandler : BaseHandler
{
public:
    CProtocolHandler() {}

    virtual std::string HandleData(void *buffer, int buffer_length, EXECUTION_CONTEXT *exec_context) = 0;

    virtual ~CProtocolHandler() {}
};

#endif