/*
   Socket.cpp

   Copyright (C) 2002-2004 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/

#include "Socket.h"
#include <iostream>
#include <unistd.h>
#include <cstring>

#define SOCKET_BIND_ERROR (-1)

#ifdef WINDOWS_OS
#include <windows.h>
#else
#define DWORD unsigned long

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
#include "ThreadUtils.h"

int Socket::nofSockets_ = 0;

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#if WINDOWS_OS
void Socket::Start()
{
    if (!nofSockets_)
    {
        WSADATA info;
        if (WSAStartup(MAKEWORD(2, 0), &info))
        {
            throw "Could not start WSA";
        }
    }
    ++nofSockets_;
}

void Socket::End()
{
    WSACleanup();
}

void Socket::Close()
{
    closesocket(s_);
}

int Socket::make_nonblocking(int file_descriptor)
{
    // Set socket to non-blocking
    u_long mode = 1; // 0 for blocking and 1 for non-blocking ; By default socket is blocking
    if (ioctlsocket(sock_file_descriptor, FIONBIO, &mode) != 0)
    {
        printf("ioctlsocket failed\n");
        closesocket(sock_file_descriptor);
        WSACleanup();
        return -1;
    }
    return 0;
}

SocketSelect::SocketSelect(Socket const *const s1, Socket const *const s2, TypeSocket type)
{
    FD_ZERO(&fds_);
    FD_SET(const_cast<Socket *>(s1)->s_, &fds_);
    if (s2)
    {
        FD_SET(const_cast<Socket *>(s2)->s_, &fds_);
    }

    TIMEVAL tval;
    tval.tv_sec = 0;
    tval.tv_usec = 1;

    TIMEVAL *ptval;
    if (type == NonBlockingSocket)
    {
        ptval = &tval;
    }
    else
    {
        ptval = 0;
    }

    if (select(0, &fds_, (fd_set *)0, (fd_set *)0, ptval) == SOCKET_ERROR)
        throw "Error in select";
}

#else
#define SOCKET int

void Socket::Start()
{
    if (!nofSockets_)
    {
        // On Linux, there is no need to initialize sockets explicitly.
        // Initialization is typically done implicitly by using socket functions.
        // You can add any necessary initialization code here if required.
    }
    ++nofSockets_;
}

void Socket::End()
{
    // On Linux, there is no equivalent to WSACleanup().
    // Cleanup of resources is typically done using close() for individual sockets.
    // You can add any necessary cleanup code here if required.
}

void Socket::Close()
{
    close(s_);
}

int Socket::make_nonblocking(int file_descriptor)
{
    int flags = fcntl(file_descriptor, F_GETFL, 0);
    if (flags == -1)
    {
        printf("fcntl(F_GETFL) failed\n");
        close(file_descriptor);
        return -1;
    }

    flags |= O_NONBLOCK;
    if (fcntl(file_descriptor, F_SETFL, flags) == -1)
    {
        printf("fcntl(F_SETFL) failed\n");
        close(file_descriptor);
        return -1;
    }

    return 0;
}

SocketSelect::SocketSelect(Socket const *const s1, Socket const *const s2, TypeSocket type)
{
    FD_ZERO(&fds_);
    FD_SET(s1->s_, &fds_);
    if (s2)
    {
        FD_SET(s2->s_, &fds_);
    }

    timeval tval;
    tval.tv_sec = 0;
    tval.tv_usec = 1;

    timeval *ptval;
    if (type == NonBlockingSocket)
    {
        ptval = &tval;
    }
    else
    {
        ptval = nullptr;
    }

    if (select(std::max(s1->s_, s2->s_) + 1, &fds_, nullptr, nullptr, ptval) == -1)
    {
        throw std::runtime_error("Error in select");
    }
}

#endif

/**
 * Method that accepts a new connection to the listening socket,
 * makes the new socket connection non blocking and returns
 * reference to the Socket object.
 */
Socket *SocketServer::Accept()
{
    SOCKET new_sock = accept(s_, 0, 0);
    if (new_sock == INVALID_SOCKET)
    {
#ifdef WINDOWS_OS
        int rc = WSAGetLastError();
        if (rc == WSAEWOULDBLOCK)
#else
        if (errno == EWOULDBLOCK || errno == EAGAIN)
#endif
        {
            return 0; // non-blocking call, no request pending
        }
        else
        {
#ifdef WINDOWS_OS
            throw "Invalid Socket";
#else
            throw std::runtime_error("Invalid Socket");
#endif
        }
    }

    make_nonblocking(new_sock);
    Socket *r = new Socket(new_sock);
    return r;
}

/**
 * Socket - Constructor
 * Creates a socket and stores the socket file descriptor in s_
 */
Socket::Socket() : s_(0)
{
    Start();
    // UDP: use SOCK_DGRAM instead of SOCK_STREAM
    s_ = socket(AF_INET, SOCK_STREAM, 0);

    if (s_ == INVALID_SOCKET)
    {
        throw "INVALID_SOCKET";
    }

    refCounter_ = new int(1);
}

/**
 * Socket - Constructor
 * Stores the socket file descriptor param in s_
 * @param s socket file descriptor
 */
Socket::Socket(SOCKET s) : s_(s)
{
    Start();
    refCounter_ = new int(1);
};

/**
 * Deconstructor
 */
Socket::~Socket()
{
    if (!--(*refCounter_))
    {
        Close();
        delete refCounter_;
    }

    --nofSockets_;
    if (!nofSockets_)
        End();
}

/**
 * Socket - Constructor
 * @param o another socket object
 */
Socket::Socket(const Socket &o)
{
    refCounter_ = o.refCounter_;
    (*refCounter_)++;
    s_ = o.s_;

    nofSockets_++;
}

/**
 * Socket - Constructor using the operator `=`
 * @param o another socket object
 */
Socket &Socket::operator=(Socket &o)
{
    (*o.refCounter_)++;

    refCounter_ = o.refCounter_;
    s_ = o.s_;

    nofSockets_++;

    return *this;
}

/**
 * Method that receives the bytes from the socket file descriptor _s .
 * Assigns the bytes received to a string and then returns it.
 */
std::string Socket::ReceiveBytes()
{
    std::string ret;
    char buf[1024];

    while (1)
    {
        u_long arg = 1024;
#ifdef WINDOWS_OS
        if (ioctlsocket(s_, FIONREAD, &arg) != 0)
#else
        if (ioctl(s_, FIONREAD, &arg) != 0)
#endif
            break;

        if (arg == 0)
            break;

        if (arg > 1024)
            arg = 1024;

        int rv = recv(s_, buf, arg, 0);
        if (rv <= 0)
            break;

        std::string t;

        t.assign(buf, rv);
        ret += t;
    }

    return ret;
}

/**
 * Receives a line from the socket file descriptor
 */
std::string Socket::ReceiveLine()
{
    std::string ret;
    while (1)
    {
        char r;

        switch (recv(s_, &r, 1, 0))
        {
        case 0: // not connected anymore;
                // ... but last line sent
                // might not end in \n,
                // so return ret anyway.
            return ret;
        case -1:
            return "";
            //      if (errno == EAGAIN) {
            //        return ret;
            //      } else {
            //      // not connected anymore
            //      return "";
            //      }
        }

        ret += r;
        if (r == '\n')
            return ret;
    }
}

/**
 * Sends a line string to the socket file descriptor
 */
void Socket::SendLine(std::string s)
{
    s += '\n';
    send(s_, s.c_str(), s.length(), 0);
}

/**
 * Sends string bytes to the socket file descriptor
 */
void Socket::SendBytes(const std::string &s)
{
    send(s_, s.c_str(), s.length(), 0);
}

/**
 * SocketServer - Constructor
 */
SocketServer::SocketServer(int port, int num_of_connections, TypeSocket type)
{

    memset(&socket_address, 0, sizeof(socket_address));

    socket_address.sin_family = PF_INET;
    socket_address.sin_port = htons(port);

    // Initialize the socket file descriptor
    // int socket(int domain, int type, int protocol)
    // AF_INET      --> ipv4
    // SOCK_STREAM  --> TCP
    // SOCK_DGRAM   --> UDP
    // protocol = 0 --> default for TCP
    s_ = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if (s_ == INVALID_SOCKET)
    {
        std::cout << "Failed to create Socket Descriptor " << std::endl;
        throw "INVALID_SOCKET";
    }

    if (type == NonBlockingSocket)
    {
        make_nonblocking(s_);
    }

    /* bind the socket to the internet address */
    if (bind(s_, (sockaddr *)&socket_address, sizeof(sockaddr_in)) == SOCKET_BIND_ERROR)
    {
        std::cout << "Failed to Bind" << std::endl;
        Close();
        throw "INVALID_SOCKET";
    }

    ;

    /**
     * Listen for connections
     */
    if (listen(s_, num_of_connections) == SOCKET_LISTEN_ERROR)
    {
        std::cout << "Listening Socket Failed.. ...." << std::endl;
        throw "LISTEN_ERROR";
    }
    else
    {
        printf("Started listening on local port : %d\n", port);
    };
}

/**
 * SocketClient - Constructor
 */
SocketClient::SocketClient(const std::string &host, int port) : Socket()
{
    std::string error;

    hostent *he;
    if ((he = gethostbyname(host.c_str())) == 0)
    {
        error = strerror(errno);
        throw error;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = *((in_addr *)he->h_addr);
    memset(&(addr.sin_zero), 0, 8);

    if (::connect(s_, (sockaddr *)&addr, sizeof(sockaddr)))
    {
#if WINDOWS_OS
        error = strerror(WSAGetLastError());
#else
        error = strerror(errno);
#endif
        throw error;
    }
}

bool SocketSelect::Readable(Socket const *const s)
{
    if (FD_ISSET(s->s_, &fds_))
        return true;
    return false;
}

int Socket::GetSocket()
{
    return this->s_;
}