#include "SocketSelect.h"

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#if WINDOWS_OS

SocketSelect::SocketSelect(Socket const *const s1, Socket const *const s2, TypeSocket type)
{
    FD_ZERO(&fds_);
    FD_SET(const_cast<Socket *>(s1)->s_, &fds_);
    if (s2)
    {
        FD_SET(const_cast<Socket *>(s2)->s_, &fds_);
    }

    TIMEVAL tval;
    tval.tv_sec = 0;
    tval.tv_usec = 1;

    TIMEVAL *ptval;
    if (type == NonBlockingSocket)
    {
        ptval = &tval;
    }
    else
    {
        ptval = 0;
    }

    if (select(0, &fds_, (fd_set *)0, (fd_set *)0, ptval) == SOCKET_ERROR)
        throw "Error in select";
}

#else

SocketSelect::SocketSelect(Socket const *const s1, Socket const *const s2, TypeSocket type)
{
    try
    {
        FD_ZERO(&fds_);
        FD_SET(s1->s_, &fds_);
        int max_fd;
        if (s2)
        {
            FD_SET(s2->s_, &fds_);
            max_fd = std::max(s1->s_, s2->s_);
        } else {
            max_fd = s1->s_;
        }

        timeval tval;
        tval.tv_sec = 0;
        tval.tv_usec = 1;

        timeval *ptval;
        if (type == NonBlockingSocket)
        {
            ptval = &tval;
        }
        else
        {
            ptval = nullptr;
        }

        if (select(max_fd + 1, &fds_, nullptr, nullptr, ptval) == -1)
        {
            std::cout << "Error during select" << std::endl;
            throw std::runtime_error("Error in select");
        }
    } catch(std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}

#endif

bool SocketSelect::Readable(Socket const *const s)
{
    if (FD_ISSET(s->s_, &fds_))
        return true;
    return false;
}