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
#include <openssl/bio.h>
#include "ThreadUtils.h"
#include "Socket.h"

class SSLSocket: public Socket {
public:
    virtual ~SSLSocket();

    SSLSocket(const SSL_METHOD *ssl_method = TLS_server_method());

    explicit SSLSocket(SOCKET s, const SSL_METHOD *ssl_method = TLS_server_method());

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

protected:
    int *refCounter_;
    SSL_CTX *ctx;
    SSL *ssl;

    void handle_error(int result);

private:
    static int nofSockets_;
    const SSL_METHOD *ssl_method;
    void Init();

    void create_context(const SSL_METHOD *method);
    void configure_client_context();
    void configure_server_context();
};


