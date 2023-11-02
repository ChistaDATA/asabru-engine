
#include "SSLSocket.h"

#include <iostream>

#ifdef WINDOWS_OS
#include <windows.h>
#else
#define DWORD unsigned long
#define u_long unsigned long

#include <sys/socket.h>
#include <stdexcept>
#include <sys/ioctl.h>
#endif

#include "ThreadUtils.h"

int SSLSocket::nofSockets_ = 0;

#define SOCKET int

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    if(!std::getenv("SSL_CERT_FILE_PATH")) {
        throw std::runtime_error("SSL_CERT_FILE_PATH environment variable is missing!");
    }

    if(!std::getenv("SSL_KEY_FILE_PATH")) {
        throw std::runtime_error("SSL_KEY_FILE_PATH environment variable is missing!");
    }

    std::string password;
    if (std::getenv("SSL_CERT_PASSPHRASE")) {
        password = std::getenv("SSL_CERT_PASSPHRASE");
    }

    // Set the userdata for the password callback.
    SSL_CTX_set_default_passwd_cb_userdata(ctx, &password);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, std::getenv("SSL_CERT_FILE_PATH"), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, std::getenv("SSL_KEY_FILE_PATH"), SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

/**
 * SSLSocket - Constructor
 * Creates a SSLSocket and stores the socket file descriptor in s_
 */
SSLSocket::SSLSocket() : s_(0)
{
    // UDP: use SOCK_DGRAM instead of SOCK_STREAM
    s_ = socket(AF_INET, SOCK_STREAM, 0);

    if (s_ == INVALID_SOCKET)
    {
        throw "INVALID_SOCKET";
    }

    refCounter_ = new int(1);
    Init();
}

/**
 * SSLSocket - Constructor
 * Stores the socket file descriptor param in s_
 * @param s socket file descriptor
 */
SSLSocket::SSLSocket(SOCKET s) : s_(s)
{
    refCounter_ = new int(1);
    Init();
};

/**
 * Deconstructor
 */
SSLSocket::~SSLSocket()
{
    if (!--(*refCounter_))
    {
        Close();
        delete refCounter_;
    }

    --nofSockets_;
}

/**
 * SSLSocket - Constructor
 * @param o another socket object
 */
SSLSocket::SSLSocket(const SSLSocket &o)
{
    refCounter_ = o.refCounter_;
    (*refCounter_)++;
    s_ = o.s_;

    nofSockets_++;
}

/**
 * SSLSocket - Constructor using the operator `=`
 * @param o another socket object
 */
SSLSocket &SSLSocket::operator=(const SSLSocket &o)
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
std::string SSLSocket::ReceiveBytes()
{
    std::string ret;
    char buf[1024];

    while (true)
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

        int rv = SSL_read(ssl,buf,arg);
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
std::string SSLSocket::ReceiveLine()
{
    std::string ret;
    while (true)
    {
        char r;

        switch (SSL_read(ssl, &r, 1))
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
void SSLSocket::SendLine(std::string s)
{
    s += '\n';
    SSL_write(ssl, s.c_str(), s.length());
}

/**
 * Sends string bytes to the socket file descriptor
 */
void SSLSocket::SendBytes(char *s, int length)
{
    std::cout << "SendBytes -- " << "SSLSocket FD : " << s_ << std::endl;
    SSL_write(ssl, s, length);
}

void SSLSocket::Close()
{
    SSL_shutdown(ssl);
    SSL_free(ssl);
    CloseSocket(s_);
}

void SSLSocket::Init()
{
    ctx = create_context();
    configure_context(ctx);

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, s_);

    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    }
}
