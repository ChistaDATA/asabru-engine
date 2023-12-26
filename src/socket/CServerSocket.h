#pragma once

#include "Socket.h"
#include "../config/EngineConstants.h"
#include "ProtocolHelper.h"
#include "load_balancer/LoadBalancerStrategy.h"
#include "load_balancer/RoundRobinStrategy.h"
#include "config_types.h"

#ifdef WINDOWS_OS
DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
DWORD WINAPI ClientThreadProc(LPVOID lpParam);
#else

// POSIX
void *ListenThreadProc(void *lpParameter);

void *ClientThreadProc(void *lpParam);

#endif


/**
 * CServerSocket
 * - This class holds the responsibility for maintaining the
 *   proxy server socket.
 */
class CServerSocket : public Socket {
    int m_ProtocolPort = 3500;
    char Protocol[255];
    int max_connections = MAX_CONNECTIONS;
    NODE_INFO info;
    struct sockaddr_in socket_address;
public:

    // Constructor
    explicit CServerSocket(
            int port,
            int num_of_connections = MAX_CONNECTIONS,
            TypeSocket type = BlockingSocket
    ) :m_ProtocolPort(port), max_connections(num_of_connections) {

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

        if (s_ == INVALID_SOCKET) {
            std::cout << "Failed to create Socket Descriptor " << std::endl;
            throw std::runtime_error("INVALID_SOCKET");
        }
        int enable = 1;
        if (setsockopt(s_, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable)) < 0) {
            std::cout << "setsockopt(SO_REUSEADDR) failed" << std::endl;
            throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
        }

        if (type == NonBlockingSocket) {
            make_nonblocking(s_);
        }
    };

    Socket *Accept();

    /** Parametrized Thread Routine */
    std::function<void *(void *)> thread_routine;

    bool StartListeningThread(const std::string &node_info, std::function<void *(void *)> pthread_routine);

    // Open the server port and start listening
    bool Open(std::string node_info, std::function<void *(void *)> pthread_routine);

#ifdef WINDOWS_OS
    static DWORD WINAPI ListenThreadProc(LPVOID lpParameter);
    static DWORD WINAPI ClientThreadProc(LPVOID lpParam, ClientTargetPair *pair);
#else

    static void *ListenThreadProc(void *lpParameter);

    static void *ClientThreadProc(void *lpParam);

#endif
};

// typedef void *(*PipelineFunction)(CServerSocket *ptr, void *lptr);

template<typename T>
using PipelineFunction = void *(*)(T *ptr, void *lptr);