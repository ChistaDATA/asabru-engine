
#include "CClientSocket.h"

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
    this->Connect();
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
        throw error;
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
        try {
            if ((m_HostPointer = gethostbyname(host.c_str())) == 0)
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
            throw error;
        }

        return true;
    } else { 
        try {
            /* Convert nnn.nnn address to a usable one */
            unsigned int m_addr = inet_addr(host.c_str());
            m_HostPointer = gethostbyaddr((char *)&m_addr, 4, AF_INET);
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
            std::cout << "Unable to get host endpoint by name " << std::endl;
            throw error;
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
