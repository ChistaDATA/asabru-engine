#include "LibuvServerSocket.h"
#include "Logger.h"
#include <utility>

// Callback function for when a new client connection is accepted
void on_new_connection(uv_stream_t *server, int status)
{
    std::cout << "on_new_connection" << std::endl;
    if (status < 0)
    {
        fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
        return;
    }
    auto curr_instance = (LibuvServerSocket *)server->data;

    // Create a client-target pair
    ClientTargetPair *pair = (ClientTargetPair *)malloc(sizeof(ClientTargetPair));

    // Initialize client and target sockets
    uv_tcp_init(curr_instance->loop, &pair->client);
    uv_tcp_init(curr_instance->loop, &pair->target);

    // Accept the incoming client connection
    if (uv_accept(server, (uv_stream_t *)&pair->client) == 0)
    {

        CLIENT_DATA clientData;
        memcpy(clientData.node_info, (char *)curr_instance->info.node_info, 255);
        clientData.mode = curr_instance->info.mode;
        clientData.ptr_to_instance = curr_instance;
        clientData.client_target_pair = pair;
        LibuvServerSocket::ClientThreadProc(&clientData);
    }
    else
    {
        uv_close((uv_handle_t *)&pair->client, NULL);
        uv_close((uv_handle_t *)&pair->target, NULL);
    }
}

/**
 * Open Socket
 */
bool LibuvServerSocket::Open(
        std::string node_info,
        std::function<void *(void *)> pipeline_thread_routine) {
    // Starts the listening thread
    return StartListeningThread(node_info, pipeline_thread_routine);
}

/**
 *  Start a Listening Thread
 */
bool LibuvServerSocket::StartListeningThread(const std::string &node_info,
                                             std::function<void *(void *)> pipeline_thread_routine) {

    LOG_INFO("Thread  => " + node_info);
    strcpy(info.node_info, node_info.c_str());
    info.mode = 1;

    this->thread_routine = pipeline_thread_routine;
    info.ptr_to_instance = (void *) this;

    if (info.ptr_to_instance == nullptr)
        return false;

#ifdef WINDOWS_OS
    DWORD Thid;
    CreateThread(NULL, 0, LibuvServerSocket::ListenThreadProc, (void *)&info, 0, &Thid);
#else
    pthread_t thread1;
    pthread_create(&thread1, nullptr, LibuvServerSocket::ListenThreadProc, (void *) &info);
#endif
    std::cout << "Started Listening Thread (LibuvServerSocket) :" << m_ProtocolPort << std::endl;
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

void *LibuvServerSocket::ListenThreadProc(
        void *lpParameter)
#endif
{
    printf("Entered the Listener Thread (LibuvServerSocket) :\n");

    NODE_INFO info;
    memcpy(&info, lpParameter, sizeof(NODE_INFO));

    std::cout << "node info => " << std::string(info.node_info) << std::endl;

    auto *curr_instance = (LibuvServerSocket *) (info.ptr_to_instance);
    if (curr_instance == nullptr) {
        std::cout << "Failed to retrieve current instance pointer" << std::endl;
        return nullptr;
    }
    curr_instance->info = info;

    std::cout << "initialize libuv loop" << std::endl;
    // Initialize libuv loop
    curr_instance->loop = static_cast<uv_loop_t *>(malloc(sizeof(uv_loop_t)));
    uv_loop_init(curr_instance->loop);

    std::cout << "initialize libuv loop end" << std::endl;

    // Create a TCP server for the proxy
    uv_tcp_t server;
    uv_tcp_init(curr_instance->loop, &server);
    server.data = curr_instance;

    // Bind and listen on a port
    uv_tcp_bind(&server, (const struct sockaddr *) &(curr_instance->socket_address), 0);
    int listen_result = uv_listen((uv_stream_t *) &server, 128, on_new_connection);

    if (listen_result)
    {
        fprintf(stderr, "Listen error: %s\n", uv_strerror(listen_result));
        return 0;
    }

    printf("Started Listening Thread : %d\n", (int)curr_instance->m_ProtocolPort);
    printf("Started listening on local port : %d\n", (int)curr_instance->m_ProtocolPort);

    // Run the libuv event loop
    uv_run(curr_instance->loop, UV_RUN_DEFAULT);
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

void *LibuvServerSocket::ClientThreadProc(
        void *threadParams)
#endif
{

    CLIENT_DATA clientData;
    memcpy(&clientData, threadParams, sizeof(CLIENT_DATA));
    ((LibuvServerSocket *) clientData.ptr_to_instance)->thread_routine((void *) &clientData);
    return 0;
}
