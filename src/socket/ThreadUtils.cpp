#include "ThreadUtils.h"

#ifdef WINDOWS_OS

//--------------- Call WSACleanUP for resource de-allocation
void Cleanup()
{
    WSACleanup();
}
//------------ Initialize WinSock Library - Initialize WSA Variables
bool StartSocket()
{
    WORD Ver;
    WSADATA wsd;
    Ver = MAKEWORD(2, 2);
    if (WSAStartup(Ver, &wsd) == SOCKET_ERROR)
    {
        WSACleanup();
        return false;
    }
    return true;
}
//-----------------Get Last Socket Error
int SocketGetLastError() { return WSAGetLastError(); }
//----------------- Close Socket
int CloseSocket(SOCKET s)
{
    closesocket(s);
    return 0;
}

/* This is the critical section object (statically allocated). */
CRITICAL_SECTION m_CriticalSection;

void InitializeLock()
{
    InitializeCriticalSection(&m_CriticalSection);
}

void AcquireLock()
{
    EnterCriticalSection(&m_CriticalSection);
}
void ReleaseLock()
{
    LeaveCriticalSection(&m_CriticalSection);
}

#else
// POSIX
#define SOCKET int

// int InComingSocket;
void Cleanup() {}
bool StartSocket() { return true; }

int SocketGetLastError() { return 0xFFFF; }
int CloseSocket(int s)
{
    shutdown(s, 2);
    return 0;
}
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#if defined(__APPLE__)
/* This is the critical section object (statically allocated). */
static pthread_mutex_t cs_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
static pthread_mutex_t cs_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

void InitializeLock()
{
}
void AcquireLock()
{
    /* Enter the critical section -- other threads are locked out */
    pthread_mutex_lock(&cs_mutex);
}
void ReleaseLock()
{
    /*Leave the critical section -- other threads can now pthread_mutex_lock()  */
    pthread_mutex_unlock(&cs_mutex);
}

#endif