#ifndef API_H
#define API_H

#include <cstdint>
#include <string>

#include "http/uri.h"
#include "http/str.h"
#include "http/request.h"
#include "http/response.h"

namespace http {
class server;
}
class preview_storage;

class api
{
public:
    explicit api(const std::shared_ptr<preview_storage> &storage) noexcept;
    ~api();

    bool start(const std::string& host, uint16_t port) noexcept;

private:
    enum method
    {
        undefined,
        post_preview,
        get_metrics
    };

    struct req_context
    {
        std::string id;
        int64_t start_ut_msecs = -1;
        int64_t duration_msecs = -1;
        int64_t width = -1;
        int64_t height = -1;
        enum method method;
    };

private:
    int handle_uri(std::shared_ptr<http::request> req, http::uri uri) noexcept;
    int handle_header(std::shared_ptr<http::request> req,
                      http::string key,
                      http::string value) noexcept;
    http::response handle_request(std::shared_ptr<http::request> req) noexcept;

    int fetch_context_from_uri(http::request_method method,
                               http::uri uri,
                               const std::shared_ptr<req_context>& cxt) const noexcept;
    int fetch_context_from_header(http::request_method method,
                                  http::string key,
                                  http::string value,
                                  const std::shared_ptr<req_context>& cxt) const noexcept;

private:
    std::shared_ptr<preview_storage> _storage;
    std::unique_ptr<http::server> _server;
};

#endif // API_H
