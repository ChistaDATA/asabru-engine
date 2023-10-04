#include "SocketServer.h"

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

    // make_nonblocking(new_sock);
    Socket *r = new Socket(new_sock);
    return r;
}