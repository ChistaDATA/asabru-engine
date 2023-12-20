#include <connection-pool/connection.h>
#include <connection-pool/pool.h>


class TestConnection final : public cpool::Connection {
public:
    bool heart_beat() override { return connected; }

    bool is_healthy() override { return connected; }

    bool connect() override {
        connected = true;
        return connected;
    }

    void disconnect() override { connected = false; }

private:
    TestConnection() = default;
    friend cpool::ConnectionPoolFactory< TestConnection >;
    bool connected = false;
};

template <>
class cpool::ConnectionPoolFactory< TestConnection > {
public:
    static std::unique_ptr< cpool::ConnectionPool > create( const std::uint16_t num_connections ) {
        std::vector< std::unique_ptr< cpool::Connection > > connections;
        for ( std::uint16_t k = 0; k < num_connections; ++k ) {
            // cannot use std::make_unique, because constructor is hidden
            connections.emplace_back( std::unique_ptr< TestConnection >( new TestConnection{} ) );
        }
        return std::unique_ptr< cpool::ConnectionPool >( new ConnectionPool{std::move( connections )} );
    }
};
