#ifndef PROTOCOL_HANDLER_H
#define PROTOCOL_HANDLER_H

#include "../socket/CServerSocket.h"
#include "BaseHandler.h"

class CProtocolHandler : BaseHandler
{
public:
    CProtocolHandler() {}

    virtual bool Handler(void *Buffer, int len, CLIENT_DATA &clientData) = 0;

    virtual ~CProtocolHandler() {}
};

#endif