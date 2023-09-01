#pragma once

#ifndef THREAD_UTILS_DOT_H
#define THREAD_UTILS_DOT_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

////////////////////////////////
#define INVALID_SOCKET (-1)
#define SOCKET_BIND_ERROR (-1)
#define SOCKET_LISTEN_ERROR (-1)
#define MAX_CONNECTIONS 5
#define SOCKET int
#define SOCKET_ERROR (-1)

void Cleanup();
bool StartSocket();
int SocketGetLastError();
int CloseSocket(SOCKET s);
void InitializeLock();
void AcquireLock();
void ReleaseLock();

#endif