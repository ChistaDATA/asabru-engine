#ifndef PROTOCOL_HELPER_DOT_H
#define PROTOCOL_HELPER_DOT_H
// UNIX/Linux-specific definitions
#define SOCKET int

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <ctype.h>
#include <functional>

#ifdef WINDOWS_OS
    #include <windows.h>
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <pthread.h>

    #define DWORD unsigned long
#endif

#define SOCKET_ERROR (-1)

class ProtocolHelper
{
public:
    static std::string GetIPAddressAsString(struct sockaddr_in *client_addr);
    static std::string GetIPPortAsString(struct sockaddr_in *client_addr);
    static bool SetReadTimeOut(SOCKET s, long second);
    static bool ReadSocketBuffer(SOCKET s, char *bfr, int size, int *num_read);
    static bool ReadSocketBuffer(SOCKET s, char *bfr, int size);
    static bool WriteSocketBuffer(SOCKET s, char *bfr, int size);
};

#endif
