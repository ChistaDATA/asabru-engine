#include "Socket.h"
#include <string>

class SocketClient : public Socket {
public:
  SocketClient(const std::string& host, int port);
};
