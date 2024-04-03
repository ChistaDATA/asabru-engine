#pragma once

#include "ProtocolHelper.h"
#include "SSLSocket.h"
#include "ThreadUtils.h"
#include <string>

/**
 * CClientSSLSocket
 * - This class holds the responsibility for maintaining the
 *   ssl client socket connection.
 */
class CClientSSLSocket : public SSLSocket {
  private:
	char m_ServerName[255];
	int m_ServerPort;
	struct sockaddr_in m_Server;
	struct hostent *m_HostPointer;
	unsigned int m_addr;
	BIO *bio;

  public:
	// Constructor
	CClientSSLSocket(std::string server_name, int client_port);
	CClientSSLSocket(SOCKET s, std::string server_name, int client_port);

	// Resolve the host name or IP address
	bool Resolve(const std::string &host);

	// Connect to the server
	void Connect();
	void SSLConnect();
	void TcpConnect();
};
