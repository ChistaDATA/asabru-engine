#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#ifdef WINDOWS_OS
#include <windows.h>
#else // else POSIX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define DWORD unsigned long

#endif

#include "Socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <iostream>
#include <functional>
#include "ThreadUtils.h"

using namespace std;

typedef struct
{
    int client_port; // Socket Handle which represents a Client
    SOCKET forward_port;   // In a non proxying mode, it will be -1
    char remote_addr[32];  //  Address of incoming endpoint
    void *ptr_to_instance; //  Pointer to the instance to which this ClientDATA belongs
    char node_info[255];   //   Node Information
    int mode;              //   R/W mode - Read Or Write
    char ProtocolName[255];
    Socket * client_socket;
} CLIENT_DATA;

#ifdef WINDOWS_OS
DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
DWORD WINAPI ClientThreadProc(LPVOID lpParam);
#else 
// POSIX
void *ListenThreadProc(void *lpParameter);
void *ClientThreadProc(void *lpParam);
#endif

typedef struct
{
    char node_info[255];   // Encoded Current Node Information as String
    int mode;              // R/W
    void * ptr_to_instance; // Pointer to CServerSocket class
} NODE_INFO;

/**
 * CServerSocket
 * - This class holds the responsiblity for maintaining the 
 *   proxy server socket. 
*/
class CServerSocket
{
    SocketServer * socket_server;
    int m_ProtocolPort = 3500;
    char Protocol[255];

public:
    SOCKET m_ListnerSocket = -1;

    NODE_INFO info;

    /** Parametrized Thread Routine */ 
    std::function<void *(void *)> thread_routine;

#ifdef WINDOWS_OS
    static DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
    static DWORD WINAPI ClientThreadProc(LPVOID lpParam);
#else
    static void *ListenThreadProc(void *lpParameter);
    static void *ClientThreadProc(void *lpParam);
#endif

    CServerSocket(int p_port, string protocol = "DEFAULT");
    bool Open(string node_info, std::function<void *(void *)> pthread_routine);
    bool StartListeningThread(string node_info, std::function<void *(void *)> pthread_routine);
    bool Close();
    bool Read(char *bfr, int size);
    bool Write(char *bfr, int size);
};

#endif
