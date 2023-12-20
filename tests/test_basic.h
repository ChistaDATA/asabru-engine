#include "TestConnection.h"
#include "../src/connection_pool/TcpConnection.h"
#include <cstdlib>
#include "unittest.h"


UNITTEST_BEGIN(testconnection)
{
    UNITTEST_SETUP();

    auto pool = cpool::ConnectionPoolFactory< TestConnection >::create( 4 );
    UNITTEST_ASSERT_INT_EQUAL( (int) pool->size_busy(), 0 );
    UNITTEST_ASSERT_INT_EQUAL( (int) pool->size_idle(), 4 );
    UNITTEST_ASSERT_INT_EQUAL( (int) pool->size(), 4 );
    
    auto connection = pool->get_connection();
    UNITTEST_ASSERT_TRUE( connection.valid() );
    UNITTEST_ASSERT_TRUE( pool->size_busy() == 1 );
    UNITTEST_ASSERT_TRUE( pool->size_idle() == 3 );
    UNITTEST_ASSERT_TRUE( pool->size() == 4 );

    auto& test_connection = dynamic_cast< TestConnection& >( *connection );
    UNITTEST_ASSERT_TRUE( test_connection.is_healthy() );

    pool->release_connection( std::move( connection ) );
    UNITTEST_ASSERT_TRUE( pool->size_busy() == 0 );
    UNITTEST_ASSERT_TRUE( pool->size_idle() == 4 );
    UNITTEST_ASSERT_TRUE( pool->size() == 4 );

    pool->heart_beat();

    UNITTEST_END();
}

//UNITTEST_BEGIN(clickhouseconnection)
//{
//    UNITTEST_SETUP();
//
//    auto pool = ConnectionPoolFactory< TcpConnection >::create( 4 , "localhost", 8123);
//    UNITTEST_ASSERT_INT_EQUAL( (int) pool->size_busy(), 0 );
//    UNITTEST_ASSERT_INT_EQUAL( (int) pool->size_idle(), 4 );
//    UNITTEST_ASSERT_INT_EQUAL( (int) pool->size(), 4 );
//
//    auto connection = pool->get_connection();
//    UNITTEST_ASSERT_TRUE( connection.valid() );
//    UNITTEST_ASSERT_TRUE( pool->size_busy() == 1 );
//    UNITTEST_ASSERT_TRUE( pool->size_idle() == 3 );
//    UNITTEST_ASSERT_TRUE( pool->size() == 4 );
//
//    auto& test_connection = dynamic_cast< TcpConnection& >( *connection );
//    UNITTEST_ASSERT_TRUE( test_connection.is_healthy() );
//
//    pool->release_connection( std::move( connection ) );
//    UNITTEST_ASSERT_TRUE( pool->size_busy() == 0 );
//    UNITTEST_ASSERT_TRUE( pool->size_idle() == 4 );
//    UNITTEST_ASSERT_TRUE( pool->size() == 4 );
//
//    pool->heart_beat();
//
//    UNITTEST_END();
//}
