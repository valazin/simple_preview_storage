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

    const char* cstr_flush_duration = std::getenv("MAP_FLUSH_DURATION_MSECS");
    if (cstr_flush_duration == nullptr) {
        return {result, false};
    }
    auto [flush_duration, is_ok] = cstr_to_int<int64_t>(cstr_flush_duration);
    if (!is_ok) {
        return {result, false};
    }
    result.map_flush_duration_msecs = static_cast<int64_t>(flush_duration);

    return {result, true};
}
