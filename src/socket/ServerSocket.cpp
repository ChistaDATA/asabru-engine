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
    info.ptr_to_instance = (void *) this;

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
DWORD WINAPI CServerSocket::ListenThreadProc(
    LPVOID lpParameter)
#else
void * CServerSocket::ListenThreadProc(
    void *lpParameter)
#endif
{

    printf("Entered the Listener Thread :\n");

    NODE_INFO info;
    memcpy(&info, lpParameter, sizeof(NODE_INFO));

    cout << "node info => " << string(info.node_info) << endl;

    CServerSocket *curr_instance = (CServerSocket *)(info.ptr_to_instance);
    if (curr_instance == 0) {
        cout << "Failed to retrieve current instance pointer" << endl;
        return 0;
    }

    SocketServer * socket_server = curr_instance->socket_server;
    
    cout << "Started listening thread loop :\n" << endl;

    while (1)
    {    

        Socket * new_sock = socket_server->Accept();
        cout << "Accepted connection :" << endl;

        CLIENT_DATA clientData;
        
        clientData.client_port = new_sock->GetSocket();
        memcpy(clientData.node_info, info.node_info, 255);

        clientData.mode = info.mode;
        string remote_ip = ProtocolHelper::GetIPAddressAsString(&(socket_server->socket_address));
        strcpy(clientData.remote_addr, remote_ip.c_str());
        cout << "Remote IP address : " << remote_ip << endl;
        string remote_port = ProtocolHelper::GetIPPortAsString(&(socket_server->socket_address));
        cout << "Remote port : " << remote_port << endl;
        clientData.ptr_to_instance = curr_instance;
        clientData.client_socket = new_sock;

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
    void *lpParam)
#endif
{

    CLIENT_DATA clientData;
    memcpy(&clientData, lpParam, sizeof(CLIENT_DATA));
    ((CServerSocket *)clientData.ptr_to_instance)->thread_routine(lpParam);
    return 0;
}
