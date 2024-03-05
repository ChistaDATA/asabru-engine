#include "LibuvProxySocket.h"
#include "../socket/SocketSelect.h"
#include "Logger.h"

void *LibuvProxySocket::ThreadHandler(LibuvProxySocket *ptr, void *lptr)
{
    CLIENT_DATA clientData;
    memcpy(&clientData, lptr, sizeof(CLIENT_DATA));

    // Retrieve the load balancer class
    auto loadBalancer = ptr->loadBalancer;

    // Check if handler is defined
    CProxyHandler *proxy_handler = ptr->GetHandler();
    if (proxy_handler == nullptr)
    {
        LOG_ERROR("The handler is not defined. Exiting!");
        return nullptr;
    }

    RESOLVED_SERVICE currentService = loadBalancer->requestServer();
    auto target_endpoint = END_POINT {
        currentService.ipaddress,
        currentService.port,
        currentService.r_w,
        currentService.alias,
        currentService.reserved,
        "  "
    };

    LOG_INFO("Resolved (Target) Host: " + target_endpoint.ipaddress);
    LOG_INFO("Resolved (Target) Port: " + std::to_string(target_endpoint.port));

    auto *client_socket = (Socket *)clientData.client_socket;
    auto *target_socket = new CClientSocket(target_endpoint.ipaddress, target_endpoint.port);

    EXECUTION_CONTEXT exec_context;

    ProtocolHelper::SetReadTimeOut(client_socket->GetSocket(), 1);
    ProtocolHelper::SetReadTimeOut(target_socket->GetSocket(), 1);

    while (true)
    {
        SocketSelect *sel;
        try
        {
            sel = new SocketSelect(client_socket, target_socket, NonBlockingSocket);
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Error occurred while creating socket select ");
            LOG_ERROR(e.what());
        }

        bool still_connected = true;
        try
        {
            if (sel->Readable(client_socket))
            {
                LOG_INFO("client socket is readable, reading bytes : ");
                std::string bytes = client_socket->ReceiveBytes();

                LOG_INFO("Calling Proxy Upstream Handler..");
                std::string response = proxy_handler->HandleUpstreamData(bytes, bytes.size(), &exec_context);
                target_socket->SendBytes((char *)response.c_str(), response.size());

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Error while sending to target ");
            LOG_ERROR(e.what());
        }

        try
        {
            if (sel->Readable(target_socket))
            {
                LOG_INFO("target socket is readable, reading bytes : ");
                std::string bytes = target_socket->ReceiveBytes();

                LOG_INFO("Calling Proxy Downstream Handler..");
                std::string response = proxy_handler->HandleDownStreamData(bytes, bytes.size(), &exec_context);
                client_socket->SendBytes((char *)response.c_str(), response.size());

                if (bytes.empty())
                    still_connected = false;
            }
        }
        catch (std::exception &e)
        {
            LOG_ERROR("Error while sending to client ");
            LOG_ERROR(e.what());
        }

        if (!still_connected)
        {
            break;
        }
    }
#ifdef WINDOWS_OS
    return 0;
#else
    return nullptr;
#endif
}
