#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#ifdef WINDOWS_OS
#include <windows.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <chrono>
#include <thread>
#endif

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

using namespace std;
#define SOCKET int
void Sleep(unsigned int microseconds);

// Class CClientSocket Definition
class CClientSocket
{
private:
    char m_ServerName[255];
    int m_PortNumber;
    struct sockaddr_in m_Server;
    struct hostent *m_HostPointer;
    unsigned int m_addr;
    SOCKET m_ConnectSock;

public:
    // Constructor
    CClientSocket(char *ServerName, int PortNumber);

    // Get the socket handle
    SOCKET GetSocket();

    // Resolve the host name or IP address
    bool Resolve();

    // Connect to the server
    bool Connect();

    // Write data to the server
    bool Write(void *buffer, int len);

    // Receive data from the server
    bool Receive(void *buffer, int *len);

    // Close the socket
    int Close();
};

#endif