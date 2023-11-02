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
#include "Socket.h"

class SSLSocket: Socket {
public:
    virtual ~SSLSocket();

    SSLSocket(const SSLSocket &);

    SSLSocket &operator=(SSLSocket const &);

    std::string ReceiveLine() override;

    std::string ReceiveBytes() override;

    // The parameter of SendLine is not a const reference
    // because SendLine modifies the std::string passed.
    void SendLine(std::string) override;

    // The parameter of SendBytes is a const reference
    // because SendBytes does not modify the std::string passed
    // (in contrast to SendLine).
    void SendBytes(char *s, int length) override;

    void Close() override;
    explicit SSLSocket(SOCKET s);
    SSLSocket();

protected:
    SOCKET s_;
    int *refCounter_;

private:
    SSL_CTX *ctx;
    SSL *ssl;
    static int nofSockets_;
    void Init();
};


