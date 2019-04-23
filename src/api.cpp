#include "api.h"

#include <iostream>

#include "http/server.h"

#include "preview_storage.h"

using namespace std::placeholders;

api::api(const std::shared_ptr<preview_storage> &storage) noexcept :
    _storage(storage)
{
    _server = std::make_unique<http::server>();
}

api::~api()
{
    _server->stop();
}

bool api::start(const std::string &host, uint16_t port) noexcept
{
    return _server->start(host, port,
                          std::bind(&api::handle_request, this, _1),
                          std::bind(&api::handle_uri, this, _1, _2),
                          std::bind(&api::handle_header, this, _1, _2, _3));
}

int api::handle_uri(std::shared_ptr<http::request> req, http::uri uri) noexcept
{
    auto cxt = std::make_shared<req_context>();

    int res = fetch_context_from_uri(req->method, uri, cxt);
    if (res == 0 || res == -1) {
        req->user_data = cxt;
    } else {
        std::cerr << "coulnd't resolve uri " << uri.to_str() << std::endl;
    }

    return res;
}

int api::handle_header(std::shared_ptr<http::request> req,
                       http::string key,
                       http::string value) noexcept
{
    auto cxt = std::reinterpret_pointer_cast<req_context>(req->user_data);
    if (!cxt) {
        std::cerr << "couldn't resolve hls context while handle header" << std::endl;
        return 500;
    }

    return fetch_context_from_header(req->method, key, value, cxt);
}

http::response api::handle_request(std::shared_ptr<http::request> req) noexcept
{
    http::response resp;

    auto cxt = std::reinterpret_pointer_cast<req_context>(req->user_data);
    if (!cxt) {
        std::cerr << "couldn't resolve hls context while handle request" << std::endl;
        resp.code = 500;
        return resp;
    }

    switch(cxt->method) {
    case method::post_preview: {
        // TODO: normal way to parse input data and return more verbose error
        std::cerr << "post preview: "
                  << cxt->id << " "
                  << cxt->start_ut_msecs << " "
                  << cxt->duration_msecs << " "
                  << cxt->width << " "
                  << cxt->height << std::endl;

        if (!cxt->id.empty()
                && cxt->start_ut_msecs > 0
                && cxt->duration_msecs > 0
                && cxt->width > 0
                && cxt->height > 0) {
            resp.code = 400;
            if (_storage->add_preview(cxt->id,
                                      cxt->start_ut_msecs,
                                      cxt->duration_msecs,
                                      cxt->width,
                                      cxt->height,
                                      req->body_buff->data(),
                                      req->body_buff->size())) {
                resp.code = 200;
            } else {
//                TODO
                resp.code = 500;
            }
        } else {
            std::cerr << "invalid one or more arugments" << std::endl;
            resp.code = 400;
        }
        break;
    }
    case method::get_metrics: {
        std::string metrics_txt;

        auto metrics = _storage->get_metrics();
        auto i = metrics.begin();
        while (i != metrics.end()) {
            metrics_txt.append(i->first + ": " + i->second + "\n");
            ++i;
        }

        resp.body_str = metrics_txt;
        break;
    }
    case method::undefined: {
        resp.code = 500;
        break;
    }
    }

    return resp;
}

int api::fetch_context_from_uri(http::request_method method,
                                http::uri uri,
                                const std::shared_ptr<api::req_context> &cxt) const noexcept
{
    auto path_items = uri.get_path_items();

    switch (method) {
    case http::request_method::options:
        std::cout << "request options " << uri.to_str();
        break;
    case http::request_method::post: {
        if (path_items.size() == 1) {
            if (path_items[0].compare("images") == 0) {
                cxt->method = method::post_preview;
                return 0;
            }
        }
        break;
    }
    case http::request_method::get:
        if (path_items.size() == 1) {
            if (path_items[0].compare("metrics") == 0) {
                cxt->method = method::get_metrics;
                return 0;
            }
        }
        break;
    case http::request_method::undefined:
        return 500;
    }

    return  404;
}

int api::fetch_context_from_header(http::request_method method,
                                   http::string key,
                                   http::string value,
                                   const std::shared_ptr<api::req_context> &cxt) const noexcept
{
    if (method != http::request_method::post) {
        return -1;
    }

    auto str_to_int64 = [cxt](http::string in, int64_t& out) -> int {
        bool ok = false;
        out = in.to_int(ok);
        if (ok) {
            return 0;
        }
        std::cerr << cxt->id << " couldn't convert " << in.to_str() << " to int64_t" << std::endl;
        return 400;
    };

    if (cxt->id.empty() && key.compare("X-STREAM-ID") == 0) {
        cxt->id = std::string(value.data(), value.size());
        return 0;
    } else if (cxt->start_ut_msecs == -1 && key.compare("X-START-TIME") == 0) {
        return str_to_int64(value, cxt->start_ut_msecs);
    } else if (cxt->duration_msecs == -1 && key.compare("X-DURATION") == 0) {
        return str_to_int64(value, cxt->duration_msecs);
    } else if (cxt->width == -1 && key.compare("X-WIDTH") == 0) {
        return str_to_int64(value, cxt->width);
    } else if (cxt->width == -1 && key.compare("X-HEIGHT") == 0) {
        return str_to_int64(value, cxt->height);
    }

    return -1;
}
