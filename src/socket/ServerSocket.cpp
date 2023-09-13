// ServerSocket.cpp - The Implementation of ServerSocket class
//
// https://github.com/eminfedar/async-sockets-cpp
//
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <functional>
#include "ProtocolHelper.h"
#include "ServerSocket.h"
#include "Socket.h"
#include "uv.h"
#include <unistd.h>

using namespace std;

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

#include "ThreadUtils.h"

// Callback function for when a new client connection is accepted
void on_new_connection(uv_stream_t *server, int status)
{
    CServerSocket *curr_instance = (CServerSocket *)server->data;
    if (status < 0)
    {
        fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
        return;
    }
    ClientTargetPair *pair;
    try
    {
        // Create a client-target pair
        pair = (ClientTargetPair *)malloc(sizeof(ClientTargetPair));
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
        cout << "Error when allocating memory" << endl;
    }

    try
    {
        // Initialize client and target sockets
        uv_tcp_init(curr_instance->loop, &pair->client);
        uv_tcp_init(curr_instance->loop, &pair->target);
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
        cout << "Error when Initializing client and target sockets" << endl;
    }

    try
    {
        // Accept the incoming client connection
        if (uv_accept(server, (uv_stream_t *)&pair->client) == 0)
        {
            CLIENT_DATA clientData;
            memcpy(clientData.node_info, (char *)curr_instance->info.node_info, 255);
            clientData.mode = curr_instance->info.mode;
            clientData.ptr_to_instance = curr_instance;
            clientData.client_target_pair = pair;
            curr_instance->ClientThreadProc(&clientData);
        }
        else
        {
            cout << "accept error" << endl;
            uv_close((uv_handle_t *)&pair->client, NULL);
            uv_close((uv_handle_t *)&pair->target, NULL);
        }
    }
    catch (std::exception &e)
    {
        cout << e.what() << endl;
    }
}

/**
 * Constructor - Initialize Local Socket
 * @param proxy_port integer
 * @param protocol Protocol
 */
CServerSocket::CServerSocket(int proxy_port, string protocol) : m_ProtocolPort(proxy_port)
{
    strcpy(Protocol, protocol.c_str());
}

/**
 * Open Socket
 */
bool CServerSocket::Open(string node_info, std::function<void *(void *)> pipeline_thread_routine)
{
    socket_server = new SocketServer(m_ProtocolPort, MAX_CONNECTIONS);
    // Starts the listening thread
    return StartListeningThread(node_info, pipeline_thread_routine);
}

/**
 *  Start a Listening Thread
 */
bool CServerSocket::StartListeningThread(string node_info, std::function<void *(void *)> pipeline_thread_routine)
{

    cout << "\nThread  => " << node_info << endl;
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

    cout << "Started Listening Thread :" << endl;
    return true;
}

/**
 * The thread that listens to incoming connections to the socket.
 * It accepts new connections and starts a new client thread.
 */
#ifdef WINDOWS_OS
DWORD WINAPI CServerSocket::ListenThreadProc(LPVOID lpParameter)
#else
void *CServerSocket::ListenThreadProc(void *lpParameter)
#endif
{
    NODE_INFO info;
    memcpy(&info, lpParameter, sizeof(NODE_INFO));

    cout << "node info => " << string(info.node_info) << endl;
    CServerSocket *curr_instance = (CServerSocket *)(info.ptr_to_instance);

    // Initialize libuv loop
    // curr_instance->loop = uv_default_loop();
    curr_instance->loop = (uv_loop_t *)malloc(sizeof(uv_loop_t));
    curr_instance->info = info;
    uv_loop_init(curr_instance->loop);

    // Create a TCP server for the proxy
    uv_tcp_t server;
    uv_tcp_init(curr_instance->loop, &server);
    server.data = curr_instance;

    // Bind and listen on a port
    uv_tcp_bind(&server, (const struct sockaddr *)&(curr_instance->socket_server->socket_address), 0);
    int listen_result = uv_listen((uv_stream_t *)&server, 128, on_new_connection);

    if (listen_result)
    {
        fprintf(stderr, "Listen error: %s\n", uv_strerror(listen_result));
        return 0;
    }

    printf("Proxy server listening on port %d\n", (int)curr_instance->m_ProtocolPort);

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
void *CServerSocket::ClientThreadProc(
    void *lpParam)
#endif
{

    CLIENT_DATA clientData;
    memcpy(&clientData, lpParam, sizeof(CLIENT_DATA));
    ((CServerSocket *)clientData.ptr_to_instance)->thread_routine(lpParam);
    return 0;
}
