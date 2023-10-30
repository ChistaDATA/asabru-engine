#pragma once

#if WINDOWS_OS
#include <WinSock2.h>
#else

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif

#include <sys/select.h>
#include <string>
#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "ThreadUtils.h"

int make_nonblocking(int socket_file_descriptor);

class SSLSocket {
public:
    virtual ~SSLSocket();

    SSLSocket(const SSLSocket &);

    SSLSocket &operator=(SSLSocket const &);

    std::string ReceiveLine();

    std::string ReceiveBytes();

    // The parameter of SendLine is not a const reference
    // because SendLine modifies the std::string passed.
    void SendLine(std::string);

    // The parameter of SendBytes is a const reference
    // because SendBytes does not modify the std::string passed
    // (in contrast to SendLine).
    void SendBytes(char *s, int length);

    int GetSocket();
    void Close();
    SSLSocket(SOCKET s);
    SSLSocket();

protected:
    SOCKET s_;
    int *refCounter_;

private:
    SSL_CTX *ctx;
    SSL *ssl;
    static int nofSockets_;

    static void Start();
    static void End();

    void Init();
};


