#pragma once

#include "../config/EngineConstants.h"
#include "ProtocolHelper.h"
#include "uv.h"

#ifdef WINDOWS_OS
DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
DWORD WINAPI ClientThreadProc(LPVOID lpParam);
#else
// POSIX
void *ListenThreadProc(void *lpParameter);
void *ClientThreadProc(void *lpParam);
#endif


/**
 * LibuvServerSocket
 * - This class holds the responsibility for maintaining the
 *   libuv proxy server socket.
 */
class LibuvServerSocket
{
    int m_ProtocolPort = 3500;
    char Protocol[255];
    int max_connections = MAX_CONNECTIONS;
    struct sockaddr_in socket_address;
public:
    // Constructor
    explicit LibuvServerSocket(
            int port,
            int num_of_connections = MAX_CONNECTIONS,
            TypeSocket type = BlockingSocket
    ): m_ProtocolPort(port),
       max_connections(num_of_connections) {
        uv_ip4_addr("0.0.0.0", port, &socket_address);
    };

    /** Parametrized Thread Routine */
    std::function<void *(void *)> thread_routine;

    bool StartListeningThread(const std::string& node_info, std::function<void *(void *)> pthread_routine);

    // Open the server port and start listening
    bool Open(std::string node_info, std::function<void *(void *)> pthread_routine);

#ifdef WINDOWS_OS
    static DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
    static DWORD WINAPI ClientThreadProc(LPVOID lpParam, ClientTargetPair *pair);
#else
    static void *ListenThreadProc(void *lpParameter);
    static void *ClientThreadProc(void *lpParam);
#endif
    uv_loop_t *loop;
    NODE_INFO info;
};

