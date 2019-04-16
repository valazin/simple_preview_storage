#ifndef ENV_SETTINGS_H
#define ENV_SETTINGS_H

#include <limits>
#include <cstdlib>
#include <string>
#include <tuple>

struct http_server_settings
{
    std::string host;
    uint16_t port = 0;
};

struct preview_storage_settings
{
    std::string path;
};

class env_settings
{
public:
    std::tuple<http_server_settings, bool>
    get_http_server_settings() const;

    std::tuple<preview_storage_settings, bool>
    get_preview_storage_settings() const;

private:
    template<class T>
    static std::tuple<T, bool> cstr_to_int(const char* cstr)
    {
        char* end = nullptr;
        long int res = strtol(cstr, &end, 10);
        if (*end != 0) {
            return {0, false};
        }
        if (res < std::numeric_limits<T>::min()
                || res > std::numeric_limits<T>::max()) {
            return {0, false};
        }
        return {res, true};
    }
};

#endif // ENV_SETTINGS_H
