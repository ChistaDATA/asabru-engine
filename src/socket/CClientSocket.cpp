
#include "CClientSocket.h"
#include "Socket.h"
#include <string>
#include <iostream>

#ifdef WINDOWS_OS
#include <windows.h>
#else
#define DWORD unsigned long
#define u_long unsigned long

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <fcntl.h> // Defines file control options. ( linux )
#include <stdexcept>
#include <sys/time.h>
#include <sys/ioctl.h>
#endif

//////////////////////////////////////////
// CClientSocket Implementation

/**
 * Constructor - Initialize Socket for client connection
 * @param proxy_port integer
 * @param protocol Protocol
 */
CClientSocket::CClientSocket(std::string server_name, int client_port) : m_ServerPort(client_port)
{
    strcpy(m_ServerName, server_name.c_str());

    // Connect to target database
    std::string error;
    hostent *he;
    try
    {
        if ((he = gethostbyname(server_name.c_str())) == 0)
        {
            std::cout << "Unable to get host endpoint by name " << std::endl;
            error = strerror(errno);
            throw std::runtime_error(error);
        }
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
        std::cout << "Unable to get host endpoint by name " << std::endl;
    }

    // Step 4
    std::cout << "IP address of " << he->h_name << " is: " << inet_ntoa(*(struct in_addr *)he->h_addr) << std::endl;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(client_port);
    addr.sin_addr = *((in_addr *)he->h_addr);
    memset(&(addr.sin_zero), 0, 8);

    if (::connect(s_, (sockaddr *)&addr, sizeof(sockaddr)))
    {

#if WINDOWS_OS
        error = strerror(WSAGetLastError());
#else
        error = strerror(errno);
#endif
        std::cout << "Unable to connect to the host endpoint : " << error << std::endl;
        throw std::runtime_error(error);
    }
}

/**
 * Open Socket
 */
bool CClientSocket::Connect()
{
    std::string host = m_ServerName;
    int port = m_ServerPort;
    std::string error;
    Resolve(host);

    // Step 4
    std::cout << "IP address of " << m_HostPointer->h_name << " is: " << inet_ntoa(*(struct in_addr *)m_HostPointer->h_addr) << std::endl;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((in_addr *)m_HostPointer->h_addr);
    memset(&(addr.sin_zero), 0, 8);

    if (::connect(s_, (sockaddr *)&addr, sizeof(sockaddr)))
    {
        std::cout << "Unable to connect to the host endpoint " << std::endl;
#if WINDOWS_OS
        error = strerror(WSAGetLastError());
#else
        error = strerror(errno);
#endif
        throw std::runtime_error(error);
    }
}

/**
 * Resolve the host name or IP address
 */
bool CClientSocket::Resolve(const std::string &host)
{
    std::string error;
    if (isalpha(host[0]))
    {
        try
        {
            if ((m_HostPointer = gethostbyname(host.c_str())) == 0)
            {
                std::cout << "Unable to get host endpoint by name " << std::endl;
                error = strerror(errno);
                throw std::runtime_error(error);
            }
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            std::cout << "Unable to get host endpoint by name " << std::endl;
            throw std::runtime_error(error);
        }

        return true;
    }
    else
    {
        try
        {
            /* Convert nnn.nnn address to a usable one */
            unsigned int m_addr = inet_addr(host.c_str());
            m_HostPointer = gethostbyaddr((char *)&m_addr, 4, AF_INET);
        }
        catch (std::exception &e)
        {
            std::cout << e.what() << std::endl;
            std::cout << "Unable to get host endpoint by name " << std::endl;
            throw std::runtime_error(error);
        }
        return true;
    }
}

// Close the socket
bool CClientSocket::Close()
{
    CloseSocket(m_ConnectSock);
    return true;
}
