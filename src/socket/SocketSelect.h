#pragma once
#include "Socket.h"
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

class SocketSelect {
// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/wsapiref_2tiq.asp
  public:
    SocketSelect(Socket const * const s1, Socket const * const s2=NULL, TypeSocket type=BlockingSocket);

    bool Readable(Socket const * const s);

  private:
    fd_set fds_;
}; 