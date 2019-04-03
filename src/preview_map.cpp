#include "preview_map.h"

#include <cstring>
#include <iostream>

preview_map::preview_map(size_t number_of_rows,
                         size_t number_of_columns,
                         size_t item_width,
                         size_t item_height) :
    c_number_of_rows(number_of_rows),
    c_number_of_columns(number_of_columns),
    c_number_of_previews(c_number_of_rows*c_number_of_columns),
    c_item_width(item_width),
    c_item_height(item_height),
    c_item_size(item_width * item_height * 3),
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

bool preview_map::insert_preview(size_t pos, char* buff, size_t size) noexcept
{
    if (size != c_item_size) {
        std::cerr << "size must be " << c_item_size
                  << " but it's " << size << std::endl;
        return false;
    }

    const size_t row_number = pos / c_number_of_columns;
    const size_t column_number = pos % c_number_of_columns;

    if (!_buff_was_allocated) {
        const size_t buff_size = c_number_of_previews * c_item_size;
        _buff = new char[buff_size];

        memset(_buff, 0, buff_size);

        _buff_was_allocated = true;
    }

    memcpy(_buff + pos*c_item_size, buff, size);

    ++_added_number_of_previews;

    return true;
}
