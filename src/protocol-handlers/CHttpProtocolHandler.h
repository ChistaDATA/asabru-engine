#pragma once

#include "BaseProtocolHandler.h"

// A request handler should expect a request as argument and returns a response
using HttpRequestHandler_t = std::function<simple_http_server::HttpResponse(const simple_http_server::HttpRequest &)>;

class CHttpProtocolHandler : public BaseProtocolHandler
{
private:
    std::map<simple_http_server::Uri, std::map<simple_http_server::HttpMethod, HttpRequestHandler_t>> request_handlers_;

public:
    CHttpProtocolHandler() {}

    void RegisterHttpRequestHandler(
            const std::string &path,
            simple_http_server::HttpMethod method,
            const HttpRequestHandler_t callback)
    {
        simple_http_server::Uri uri(path);
        request_handlers_[uri].insert(std::make_pair(method, std::move(callback)));
    }

    void RegisterHttpRequestHandler(const simple_http_server::Uri &uri, simple_http_server::HttpMethod method,
                                    const HttpRequestHandler_t callback)
    {
        request_handlers_[uri].insert(std::make_pair(method, std::move(callback)));
    }

    virtual std::string HandleData(std::string request_string, unsigned long buffer_length, EXECUTION_CONTEXT *exec_context);
    simple_http_server::HttpResponse HandleHttpRequest(const simple_http_server::HttpRequest &request);

    ~CHttpProtocolHandler() {}
};