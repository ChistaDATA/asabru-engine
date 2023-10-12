#pragma once
#include <sys/socket.h>
#include "../socket/Socket.h"

#define SOCKET int

typedef struct
{
    int client_port;       // Socket Handle which represents a Client
    SOCKET forward_port;   // In a non proxying mode, it will be -1
    char remote_addr[32];  //  Address of incoming endpoint
    void *ptr_to_instance; //  Pointer to the instance to which this ClientDATA belongs
    char node_info[255];   //   Node Information
    int mode;              //   R/W mode - Read Or Write
    char ProtocolName[255];
    Socket * client_socket;
    int current_service_index;
} CLIENT_DATA;

typedef struct
{
    char node_info[255];   // Encoded Current Node Information as String
    int mode;              // R/W
    void * ptr_to_instance; // Pointer to CServerSocket class
} NODE_INFO;
