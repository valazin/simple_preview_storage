#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

class string_utils
{
public:
    static std::vector<std::string> split_string(const std::string &text,
                                                 char delimiter) noexcept;
    static bool string_is_number(const std::string &text) noexcept;
};

#endif // STRING_UTILS_H
