#include "CHttpProtocolHandler.h"
#include "logger/Logger.h"
#include <regex>

simple_http_server::HttpResponse
CHttpProtocolHandler::HandleHttpRequest(const simple_http_server::HttpRequest &request) {
    std::map<simple_http_server::Uri, std::map<simple_http_server::HttpMethod, HttpRequestHandler_t>>::iterator it;
    std::string request_path = request.uri().path();
    for (it = request_handlers_.begin(); it != request_handlers_.end(); ++it) {
        std::string path = it->first.path();
        const std::regex pattern(path, std::regex_constants::icase);
        if (regex_match(request_path.begin(), request_path.end(), pattern)) {
            break;
        }
    }
    if (it == request_handlers_.end()) { // this uri is not registered
        simple_http_server::HttpResponse not_found_response = simple_http_server::HttpResponse(
                simple_http_server::HttpStatusCode::NotFound);
        not_found_response.SetHeader("Content-Length", "0");
        return not_found_response;
    }
    auto callback_it = it->second.find(request.method());
    if (callback_it == it->second.end()) { // no handler for this method
        simple_http_server::HttpResponse method_not_allowed_response = simple_http_server::HttpResponse(
                simple_http_server::HttpStatusCode::MethodNotAllowed);
        method_not_allowed_response.SetHeader("Content-Length", "0");
        return method_not_allowed_response;
    }
    return callback_it->second(request); // call handler to process the request
}

/**
 * Function that handles data
 * @param request_string - the buffer that we receive from client
 * @param buffer_length - length of the buffer
 */
std::string
CHttpProtocolHandler::HandleData(std::string buffer, unsigned long buffer_length, EXECUTION_CONTEXT *exec_context) {
    simple_http_server::HttpRequest http_request;
    simple_http_server::HttpResponse http_response;

    try {
        std::string request_string = buffer;
        request_string = request_string.substr(0, buffer_length);
        if (request_string.empty()) {
            LOG_ERROR("Empty request string");
            throw std::invalid_argument("Empty request string");
        }
        http_request = simple_http_server::string_to_request(request_string);
        http_response = HandleHttpRequest(http_request);
    }
    catch (const std::invalid_argument &e) {
        http_response = simple_http_server::HttpResponse(simple_http_server::HttpStatusCode::BadRequest);
        http_response.SetContent(e.what());
    }
    catch (const std::logic_error &e) {
        http_response = simple_http_server::HttpResponse(simple_http_server::HttpStatusCode::HttpVersionNotSupported);
        http_response.SetContent(e.what());
    }
    catch (const std::exception &e) {
        http_response = simple_http_server::HttpResponse(simple_http_server::HttpStatusCode::InternalServerError);
        http_response.SetContent(e.what());
    }

    // Set response to write to client
    std::string response_string =
            to_string(http_response, http_request.method() != simple_http_server::HttpMethod::HEAD);
    return response_string;
}

extern "C" CHttpProtocolHandler *createCHttpProtocolHandler() {
    return new CHttpProtocolHandler;
}

extern "C" void destroyCHttpProtocolHandler(CHttpProtocolHandler *c) {
    delete c;
}
