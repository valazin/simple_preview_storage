#include "string_utils.h"

std::vector<std::string> string_utils::split_string(const std::string &text, 
                                                    char delimiter) noexcept 
{
    std::vector<std::string> strList;
    size_t prevPos=0;
    for(size_t pos=0; pos<text.size(); ++pos) {
        if (text[pos]==delimiter) {
            strList.push_back(std::string(text, prevPos, pos-prevPos));
            prevPos=pos+1;
        }
    }
    if (prevPos>0) {
        strList.push_back(std::string(text, prevPos, text.size()-prevPos));
    }
    return strList;
}

bool string_utils::string_is_number(const std::string& text) noexcept
{
    std::string_view text_view(text);
    if (text_view.empty()) {
        return false;
    }

    if (text_view.front() == '-') {
        if (text_view.size() >1 ) {
            text_view.remove_prefix(1);
        } else {
            return false;
        }
    }

    bool contains_only_number = text_view.find_first_not_of("0123456789") == std::string::npos;
    return contains_only_number;
}
