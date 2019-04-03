#include "preview_map.h"

#include <cstring>
#include <iostream>

preview_map::preview_map(size_t number_of_rows,
                         size_t number_of_columns,
                         size_t preview_width,
                         size_t preview_height) noexcept :
    c_number_of_rows(number_of_rows),
    c_number_of_columns(number_of_columns),
    c_number_of_previews(c_number_of_rows*c_number_of_columns),
    c_preview_width(preview_width),
    c_preview_height(preview_height),
    c_preview_size(preview_width * preview_height * 3),
    c_map_size(c_preview_size * c_number_of_previews),
    _preview_info(c_number_of_previews, preview_info())
{
}

preview_map::~preview_map()
{
    if (_buff != nullptr) {
        delete[] _buff;
    }
}

bool preview_map::is_full() const noexcept
{
    return (_added_number_of_previews == c_number_of_previews);
}

char *preview_map::data() const noexcept
{
    return _buff;
}

size_t preview_map::size() const noexcept
{
    return c_map_size;
}

bool preview_map::insert_preview(size_t pos, char* buff, size_t size) noexcept
{
    if (size != c_preview_size) {
        std::cerr << "size must be " << c_preview_size
                  << " but it's " << size << std::endl;
        return false;
    }

    const size_t row_number = pos / c_number_of_columns;
    const size_t column_number = pos % c_number_of_columns;

    if (!_buff_was_allocated) {
        _buff = new char[c_map_size];

        memset(_buff, 0, c_map_size);

        _buff_was_allocated = true;
    }

    memcpy(_buff + pos*c_preview_size, buff, size);

    ++_added_number_of_previews;

    return true;
}
