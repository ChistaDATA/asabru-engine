//
// Created by Midhun Darvin on 02/11/23.
//

#include "CClientSSLSocket.h"

#include <iostream>
#include <string>

#ifdef WINDOWS_OS
#include <windows.h>
#else
#define DWORD unsigned long
#define u_long unsigned long

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/socket.h>
#endif

//////////////////////////////////////////
// CClientSSLSocket Implementation

/**
 * Constructor - Initialize Socket for client connection
 * @param proxy_port integer
 * @param protocol Protocol
 */
CClientSSLSocket::CClientSSLSocket(std::string server_name, int client_port)
	: m_ServerPort(client_port), SSLSocket(TLS_client_method()) {
	strcpy(m_ServerName, server_name.c_str());
	std::string error;

	Connect();
}

CClientSSLSocket::CClientSSLSocket(SOCKET s, std::string server_name, int client_port)
	: m_ServerPort(client_port), SSLSocket(s, TLS_client_method()) {
	strcpy(m_ServerName, server_name.c_str());
	SSLConnect();
}

void CClientSSLSocket::TcpConnect() {
	std::string host = m_ServerName;
	int port = m_ServerPort;
	std::string error;
	Resolve(host);

	// Step 4
	std::cout << "IP address of " << m_HostPointer->h_name << " is: " << inet_ntoa(*(struct in_addr *)m_HostPointer->h_addr)
			  << std::endl;

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr = *((in_addr *)m_HostPointer->h_addr);
	memset(&(addr.sin_zero), 0, 8);

	if (::connect(s_, (sockaddr *)&addr, sizeof(sockaddr))) {
		std::cout << "Unable to connect to the host endpoint " << std::endl;
#if WINDOWS_OS
		error = strerror(WSAGetLastError());
#else
		error = strerror(errno);
#endif
		throw std::runtime_error(error);
	}

	printf("TCP connection to server successful\n");
}

/**
 * Open Socket
 */
void CClientSSLSocket::Connect() {
	TcpConnect();
	SSLConnect();
}

void CClientSSLSocket::SSLConnect() {
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, s_);

	/*
	 * Tell the server during the handshake which hostname we are attempting
	 * to connect to in case the server supports multiple hosts.
	 */
	if (!SSL_set_tlsext_host_name(ssl, m_ServerName)) {
		handle_error(EXIT_FAILURE);
		throw std::runtime_error("Failed to set the SNI hostname\n");
	}

	/*
	 * Ensure we check during certificate verification that the server has
	 * supplied a certificate for the hostname that we were expecting.
	 * Virtually all clients should do this unless you really know what you
	 * are doing.
	 */
	if (!SSL_set1_host(ssl, m_ServerName)) {
		handle_error(EXIT_FAILURE);
		throw std::runtime_error("Failed to set the certificate verification hostname");
	}

	/* Now do SSL connect with server */
	if (SSL_connect(ssl) == 1) {

		printf("SSL connection to server successful\n");
		printf("SSL connected using %s\n", SSL_get_cipher(ssl));

	} else {
		handle_error(EXIT_FAILURE);
		throw std::runtime_error("SSL connection to server failed\n");
	}
}

/**
 * Resolve the host name or IP address
 */
bool CClientSSLSocket::Resolve(const std::string &host) {
	std::string error;
	if (isalpha(host[0])) {
		try {
			if ((m_HostPointer = gethostbyname(host.c_str())) == nullptr) {
				std::cout << "Unable to get host endpoint by name " << std::endl;
				error = strerror(errno);
				throw std::runtime_error(error);
			}
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
			std::cout << "Unable to get host endpoint by name " << std::endl;
			throw std::runtime_error(error);
		}

		return true;
	} else {
		try {
			/* Convert nnn.nnn address to a usable one */
			unsigned int m_addr = inet_addr(host.c_str());
			m_HostPointer = gethostbyaddr((char *)&m_addr, 4, AF_INET);
		} catch (std::exception &e) {
			std::cout << e.what() << std::endl;
			std::cout << "Unable to get host endpoint by name " << std::endl;
			throw std::runtime_error(error);
		}
		return true;
	}
}
