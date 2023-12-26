#pragma once
#include "Socket.h"
#include <string>
#include "ProtocolHelper.h"
#include "ThreadUtils.h"

#define SOCKET int
void Sleep(unsigned int microseconds);

/**
 * CClientSocket
 * - This class holds the responsibility for maintaining the
 *   client socket connection. 
*/
class CClientSocket : public Socket
{
private:
    int m_ServerPort;
    char m_ServerName[255];
    struct sockaddr_in m_Server;
    struct hostent *m_HostPointer;
    unsigned int m_addr;

public:
    // Constructor
    CClientSocket(std::string server_name, int client_port);

    // Resolve the host name or IP address
    bool Resolve(const std::string &host);

    // Connect to the server
    void Connect();

    // Reconnect to the server
    void Reconnect();

    ~CClientSocket() { Close(); }
};
