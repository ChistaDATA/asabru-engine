#include <connection-pool/connection.h>
#include <connection-pool/pool.h>
#include "socket/CClientSocket.h"

class TcpConnection final : public cpool::Connection
{
public:
    int connection_id;
    CClientSocket *client_socket = nullptr;

    bool heart_beat() override {
        return connected;
    }

    bool is_healthy() override {
        return false;
    }

    bool connect() override
    {
        std::cout << "connect::" << std::endl;
        std::cout << "connection_id::" << connection_id << std::endl;
        client_socket->Reconnect();
        connected = true;
        return connected;
    }

    void disconnect() override
    {
        if (client_socket)
        {
            client_socket->Close();
        }
        delete client_socket;
        client_socket = nullptr;
        connected = false;
        std::cout << "Disconnected!" << std::endl;
        std::cout << "connection_id::" << connection_id << std::endl;
    }

    ~TcpConnection()
    {
        std::cout << "TcpConnection::destructor" << std::endl;
        std::cout << "connection_id::" << connection_id << std::endl;
        disconnect();
    }

private:
    std::string server_name;
    int server_port;

    TcpConnection(int connection_id, std::string server_name, int server_port) : connection_id(connection_id), server_name(server_name), server_port(server_port)
    {
        std::cout << "TcpConnection::constructor" << std::endl;
        std::cout << "connection_id::" << connection_id << std::endl;
        client_socket = new CClientSocket(server_name, server_port);
    };
    friend cpool::ConnectionPoolFactory<TcpConnection>;
    bool connected = false;
};

template <>
class cpool::ConnectionPoolFactory<TcpConnection>
{
public:
    static std::unique_ptr<cpool::ConnectionPool> create(const std::uint16_t num_connections, std::string server_name, int server_port)
    {
        std::vector<std::unique_ptr<cpool::Connection>> connections;
        for (std::uint16_t k = 0; k < num_connections; ++k)
        {
            // cannot use std::make_unique, because constructor is hidden
            connections.emplace_back(std::unique_ptr<TcpConnection>(new TcpConnection(k, server_name, server_port)));
        }
        return std::unique_ptr<cpool::ConnectionPool>(new cpool::ConnectionPool{std::move(connections)});
    }
};
