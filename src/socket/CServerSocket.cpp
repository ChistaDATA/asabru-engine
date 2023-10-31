// ServerSocket.cpp - The Implementation of ServerSocket class
//
// https://github.com/eminfedar/async-sockets-cpp
//
//
#include "CServerSocket.h"
#include <utility>

/**
 * CServerSocket - Constructor
 */
CServerSocket::CServerSocket(int port, int num_of_connections, TypeSocket type) : m_ProtocolPort(port), max_connections(num_of_connections)
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
        throw std::runtime_error("INVALID_SOCKET");
    }

    if (type == NonBlockingSocket)
    {
        make_nonblocking(s_);
    }
}

/**
 * Open Socket
 */
bool CServerSocket::Open(
    std::string node_info,
    std::function<void *(void *)> pipeline_thread_routine)
{
    /* bind the socket to the internet address */
    if (bind(s_, (sockaddr *)&socket_address, sizeof(sockaddr_in)) == SOCKET_BIND_ERROR)
    {
        std::cout << "Failed to Bind" << std::endl;
        Close();
        throw std::runtime_error("INVALID_SOCKET");
    };

    /**
     * Listen for connections
     */
    if (listen(s_, max_connections) == SOCKET_LISTEN_ERROR)
    {
        std::cout << "Listening Socket Failed.. ...." << std::endl;
        throw std::runtime_error("LISTEN_ERROR");
    }
    else
    {
        printf("Started listening on local port : %d\n", m_ProtocolPort);
    };

    // Starts the listening thread
    return StartListeningThread(node_info, pipeline_thread_routine);
}

/**
 *  Start a Listening Thread
 */
bool CServerSocket::StartListeningThread(const std::string& node_info, std::function<void *(void *)> pipeline_thread_routine)
{

    std::cout << "\nThread  => " << node_info << std::endl;
    strcpy(info.node_info, node_info.c_str());
    info.mode = 1;

    this->thread_routine = pipeline_thread_routine;
    info.ptr_to_instance = (void *)this;

    if (info.ptr_to_instance == 0)
        return false;

#ifdef WINDOWS_OS
    DWORD Thid;
    CreateThread(NULL, 0, CServerSocket::ListenThreadProc, (void *)&info, 0, &Thid);
#else
    pthread_t thread1;
    int iret1 = pthread_create(&thread1, NULL, CServerSocket::ListenThreadProc, (void *)&info);
#endif

    std::cout << "Started Listening Thread :" << m_ProtocolPort << std::endl;
    return true;
}

/**
 * The thread that listens to incoming connections to the socket.
 * It accepts new connections and starts a new client thread.
 */
#ifdef WINDOWS_OS
DWORD WINAPI CServerSocket::ListenThreadProc(
    LPVOID lpParameter)
#else
void *CServerSocket::ListenThreadProc(
    void *lpParameter)
#endif
{
    printf("Entered the Listener Thread :\n");

    NODE_INFO info;
    memcpy(&info, lpParameter, sizeof(NODE_INFO));
    int current_service_index = 0;

    std::cout << "node info => " << std::string(info.node_info) << std::endl;

    CServerSocket *curr_instance = (CServerSocket *)(info.ptr_to_instance);
    if (curr_instance == 0)
    {
        std::cout << "Failed to retrieve current instance pointer" << std::endl;
        return 0;
    }

    // CServerSocket *socket_server = curr_instance->socket_server;
    std::cout << "Started listening thread loop :\n"
              << std::endl;


    while (1)
    {
        Socket *new_sock = curr_instance->Accept();
        std::cout << "Accepted connection :" << std::endl;

        CLIENT_DATA clientData;
        clientData.client_port = new_sock->GetSocket();
        memcpy(clientData.node_info, info.node_info, 255);
        clientData.mode = info.mode;
        std::string remote_ip = ProtocolHelper::GetIPAddressAsString(&(curr_instance->socket_address));
        strcpy(clientData.remote_addr, remote_ip.c_str());
        std::cout << "Remote IP address : " << remote_ip << std::endl;
        std::string remote_port = ProtocolHelper::GetIPPortAsString(&(curr_instance->socket_address));
        std::cout << "Remote port : " << remote_port << std::endl;
        clientData.ptr_to_instance = curr_instance;
        clientData.client_socket = new_sock;

        /**
         * We need to configure client data, such that the new connection is made to a target endpoint
         * based on a algorithm. For eg. round robin
         *
         * If there are multiple services in endpoint configuration, we would choose a target service based on a round robin manner.
         * ( TODO: The algorithm used here would be configurable. )
         *
         * If there is only one service in the endpoint configuration, then we can directly choose the target service.
         */
        clientData.current_service_index = current_service_index;
        current_service_index = ( current_service_index + 1 ) % 100;

#ifdef WINDOWS_OS
        DWORD ThreadId;
        ::CreateThread(NULL, 0, CServerSocket::ClientThreadProc, (LPVOID)&ClientData, 0, &ThreadId);
#else
        pthread_t thread2;
        pthread_create(&thread2, NULL, CServerSocket::ClientThreadProc, (void *)&clientData);
#endif
        usleep(3000);
    }

    return 0;
}

/**
 * Creates a client thread procedure that handles incoming and outgoing
 * messages to the socket. It offloads the work to the handler
 */
#ifdef WINDOWS_OS
DWORD WINAPI CServerSocket::ClientThreadProc(
    LPVOID lpParam)
#else
void *CServerSocket::ClientThreadProc(
    void *threadParams)
#endif
{

    CLIENT_DATA clientData;
    memcpy(&clientData, threadParams, sizeof(CLIENT_DATA));
    ((CServerSocket *)clientData.ptr_to_instance)->thread_routine((void *)&clientData);
    return 0;
}

/**
 * Method that accepts a new connection to the listening socket,
 * makes the new socket connection non blocking and returns
 * reference to the Socket object.
 */
Socket *CServerSocket::Accept()
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