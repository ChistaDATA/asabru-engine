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
#include "ThreadUtils.h"

int Socket::nofSockets_ = 0;

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

#endif

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
void Socket::SendBytes(char *s, int length)
{
    std::cout << "SendBytes -- " << "Socket FD : " << s_ << std::endl;
    send(s_, s, length, 0);
}

int Socket::GetSocket()
{
    return this->s_;
}

void Socket::Close()
{
    CloseSocket(s_);
}