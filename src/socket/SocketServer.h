#pragma once
#include "Socket.h"

class SocketServer : public Socket {
public:
  struct sockaddr_in socket_address;
  SocketServer(int port, int num_of_connections, TypeSocket type=BlockingSocket);

  Socket* Accept();
};