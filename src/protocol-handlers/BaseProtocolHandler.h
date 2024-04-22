#pragma once

#include "CommonTypes.h"
#include "LineGrabber.h"
#include "Utils.h"
#include "http/http_message.h"
#include "interface/CProtocolHandler.h"
#include "socket/CServerSocket.h"
#include <map>

class BaseProtocolHandler : public CProtocolHandler
{
public:
    /**
     * Function that handles upstream data
     * @param buffer - the buffer that we receive from upstream ( source dbs )
     * @param length - length of the buffer
     */
    std::string HandleData(void *request_string, unsigned long buffer_length, EXECUTION_CONTEXT *exec_context)
    {
        std::string response = "";
        response += "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/plain\r\n";
        response += "Content-Length: 12\r\n";
        response += "\r\n";
        response += "Hello World!";

        size_t response_length = strlen(response.c_str());
        char *result = (char *)malloc(response_length);
        memcpy(result, response.c_str(), response_length);

        return result;
    }
};
