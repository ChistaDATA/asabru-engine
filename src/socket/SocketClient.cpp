#include "Socket.h"
#include "SocketClient.h"
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

/**
 * SocketClient - Constructor
 */
SocketClient::SocketClient(const std::string &host, int port) : Socket()
{
    std::string error;
    hostent *he;
    try
    {
        if ((he = gethostbyname(host.c_str())) == 0)
        {
            std::cout << "Unable to get host endpoint by name " << std::endl;
            error = strerror(errno);
            throw error;
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
    addr.sin_port = htons(port);
    addr.sin_addr = *((in_addr *)he->h_addr);
    memset(&(addr.sin_zero), 0, 8);

    if (::connect(s_, (sockaddr *)&addr, sizeof(sockaddr)))
    {
        std::cout << "Unable to connect to the host endpoint " << std::endl;
#if WINDOWS_OS
        error = strerror(WSAGetLastError());
#else
        error = strerror(errno);
#endif
        throw error;
    }
}
