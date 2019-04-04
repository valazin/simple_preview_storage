#ifndef URI_H
#define URI_H

#include <string>

#include "str.h"

namespace http {

typedef std::pair<string, string> query;

class uri
{
public:
    uri() noexcept;
    uri(const char *buff, size_t size) noexcept;

    bool is_valid() const;

    std::vector<string> get_path_items() const noexcept;
    std::vector<query> get_query_items() const noexcept;

    string find_query_item(const char *key) const noexcept;

    std::string to_str() const noexcept;

private:
    bool _is_valid = false;

    const char* _buff = nullptr;
    size_t _size = 0;

    std::vector<string> _path_items;
    std::vector<query> _query_items;
};

}

#endif // URI_H
