#include "env_settings.h"

std::tuple<http_server_settings, bool>
env_settings::get_http_server_settings() const
{
    http_server_settings result;

    const char* cstr_host = std::getenv("HTTP_HOST");
    if (cstr_host == nullptr) {
        return {result, false};
    }
    result.host = std::string(cstr_host);

    const char* cstr_port = std::getenv("HTTP_PORT");
    if (cstr_port == nullptr) {
        return {result, false};
    }
    auto [port, is_ok] = cstr_to_int<uint16_t>(cstr_port);
    if (!is_ok) {
        return {result, false};
    }
    result.port = static_cast<uint16_t>(port);

    return {result, true};
}

std::tuple<preview_storage_settings, bool>
env_settings::get_preview_storage_settings() const
{
    preview_storage_settings result;

    const char* cstr_path = std::getenv("ARCHIVE_PATH");
    if (cstr_path == nullptr) {
        return {result, false};
    }
    result.path = std::string(cstr_path);

    const char* cstr_flush_duration = std::getenv("MAP_FLUSH_DURATION_SECS");
    if (cstr_flush_duration == nullptr) {
        return {result, false};
    }

    if (auto [flush_duration, is_ok] = cstr_to_int<int64_t>(cstr_flush_duration); is_ok) {
        result.map_flush_duration_secs = static_cast<int64_t>(flush_duration);
    } else {
        return {result, false};
    }

    const char* cstr_release_timeout = std::getenv("MAP_RELEASE_TIMEOUT_SECS");
    if (cstr_release_timeout == nullptr) {
        return {result, false};
    }
    if (auto [release_timeout, is_ok] = cstr_to_int<int64_t>(cstr_release_timeout); is_ok) {
        result.map_release_timeout_secs = release_timeout;
    } else {
        return {result, false};
    }

    return {result, true};
}
