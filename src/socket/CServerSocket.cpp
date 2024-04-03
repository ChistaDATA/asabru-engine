// ServerSocket.cpp - The Implementation of ServerSocket class
//
// https://github.com/eminfedar/async-sockets-cpp
//
//
#include "CServerSocket.h"
#include "Logger.h"
#include "ThreadPoolSingleton.h"
#include "load_balancer/LoadBalancer.h"
#include <sstream>
#include <utility>

/**
 * Open Socket
 */
bool CServerSocket::Open(std::string node_info, std::function<void *(void *)> pipeline_thread_routine) {
	/* bind the socket to the internet address */
	if (::bind(s_, (sockaddr *)&socket_address, sizeof(sockaddr_in)) == SOCKET_BIND_ERROR) {
		LOG_ERROR("Failed to Bind");
		Close();
		throw std::runtime_error("INVALID_SOCKET");
	};

	/**
	 * Listen for connections
	 */
	if (listen(s_, max_connections) == SOCKET_LISTEN_ERROR) {
		LOG_ERROR("Listening Socket Failed.. ....");
		throw std::runtime_error("LISTEN_ERROR");
	} else {
		LOG_INFO("Started listening on local port : " + std::to_string(m_ProtocolPort));
	};

	// Starts the listening thread
	return StartListeningThread(node_info, pipeline_thread_routine);
}

/**
 *  Start a Listening Thread
 */
bool CServerSocket::StartListeningThread(const std::string &node_info, std::function<void *(void *)> pipeline_thread_routine) {

	LOG_INFO("Thread  => " + node_info);
	strcpy(info.node_info, node_info.c_str());
	info.mode = 1;

	this->thread_routine = pipeline_thread_routine;
	info.ptr_to_instance = (void *)this;

	if (info.ptr_to_instance == nullptr)
		return false;

#ifdef WINDOWS_OS
	DWORD Thid;
	CreateThread(NULL, 0, CServerSocket::ListenThreadProc, (void *)&info, 0, &Thid);
#else
	pthread_t thread1;
	pthread_create(&thread1, nullptr, CServerSocket::ListenThreadProc, (void *)&info);
#endif

	LOG_INFO("Started Listening Thread :" + std::to_string(m_ProtocolPort));
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
	LOG_INFO("Entered the Listener Thread :");

	NODE_INFO info;
	memcpy(&info, lpParameter, sizeof(NODE_INFO));
	LOG_INFO("node info => " + std::string(info.node_info));

	auto *curr_instance = (CServerSocket *)(info.ptr_to_instance);
	if (curr_instance == nullptr) {
		LOG_ERROR("Failed to retrieve current instance pointer");
		return nullptr;
	}

	LOG_INFO("Started listening thread loop :");
	while (true) {
		Socket *new_sock = curr_instance->Accept();
		LOG_INFO("Accepted connection :");

		/**
		 * Configure client connection details
		 */
		CLIENT_DATA clientData;
		clientData.client_port = new_sock->GetSocket();
		memcpy(clientData.node_info, info.node_info, 255);
		clientData.mode = info.mode;
		std::string remote_ip = ProtocolHelper::GetIPAddressAsString(&(curr_instance->socket_address));
		strcpy(clientData.remote_addr, remote_ip.c_str());
		LOG_INFO("Remote IP address : " + remote_ip);
		std::string remote_port = ProtocolHelper::GetIPPortAsString(&(curr_instance->socket_address));
		LOG_INFO("Remote port : " + remote_port);
		clientData.ptr_to_instance = curr_instance;
		clientData.client_socket = new_sock;

		/**
		 * Push the request handler as a Task to the
		 * thread pool.
		 */
		ThreadPoolSingleton::pool.push({
			thread_pool::TaskType::Execute, // TaskType
			[&clientData](std::vector<thread_pool::Param> const &params) {
				std::ostringstream ss;
				ss << std::this_thread::get_id();
				LOG_INFO("Thread ID: " + ss.str() + " request");
				CServerSocket::ClientThreadProc((void *)&clientData);
			},
			{} // Arguments
		});
		usleep(3000);
	}

	return 0;
}

/**
 * Creates a client thread procedure that handles incoming and outgoing
 * messages to the socket. It offloads the work to the handler
 */
#ifdef WINDOWS_OS
DWORD WINAPI CServerSocket::ClientThreadProc(LPVOID lpParam)
#else

void *CServerSocket::ClientThreadProc(void *threadParams)
#endif
{

	CLIENT_DATA clientData;
	memcpy(&clientData, threadParams, sizeof(CLIENT_DATA));
	try {
		((CServerSocket *)clientData.ptr_to_instance)->thread_routine((void *)&clientData);
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}

	return nullptr;
}

/**
 * Method that accepts a new connection to the listening socket,
 * makes the new socket connection non blocking and returns
 * reference to the Socket object.
 */
Socket *CServerSocket::Accept() {
	SOCKET new_sock = accept(s_, 0, 0);
	if (new_sock == INVALID_SOCKET) {
#ifdef WINDOWS_OS
		int rc = WSAGetLastError();
		if (rc == WSAEWOULDBLOCK)
#else
		if (errno == EWOULDBLOCK || errno == EAGAIN)
#endif
		{
			return nullptr; // non-blocking call, no request pending
		} else {
#ifdef WINDOWS_OS
			throw "Invalid Socket";
#else
			throw std::runtime_error("Invalid Socket");
#endif
		}
	}

	// make_nonblocking(new_sock);
	auto *r = new Socket(new_sock);
	return r;
}
