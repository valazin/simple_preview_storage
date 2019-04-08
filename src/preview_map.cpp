#include "preview_map.h"

#include <cstring>
#include <iostream>

preview_map::preview_map(size_t number_of_rows,
                         size_t number_of_columns,
                         size_t preview_width,
                         size_t preview_height) noexcept :
    _rows(number_of_rows),
    _cols(number_of_columns),
    _number_of_previews(_rows*_cols),
    _preview_width(preview_width),
    _preview_height(preview_height),
    _preview_size(preview_width * preview_height * 3),
    _map_size(_preview_size * _number_of_previews),
    _preview_info(_number_of_previews, preview_info())
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
    return (_added_number_of_previews == _number_of_previews);
}

char *preview_map::data() const noexcept
{
    return _buff;
}

size_t preview_map::size() const noexcept
{
    return _map_size;
}

size_t preview_map::width_px() const
{
    return _cols * _preview_width;
}

size_t preview_map::height_px() const
{
    return _rows * _preview_height;
}

bool preview_map::insert_preview(size_t number,
                                 const char* buff,
                                 std::size_t size) noexcept
{
    if (size != _preview_size) {
        std::cerr << "size must be " << _preview_size
                  << " but it's " << size << std::endl;
        return false;
    }

    const size_t row = number / _cols;
    const size_t col = number % _cols;

    if (!_buff_was_allocated) {
        _buff = new char[_map_size];

        memset(_buff, 0, _map_size);

        _buff_was_allocated = true;
    }

    size_t pos = (3 * col * _preview_width);
    if (row > 0) {
        pos += 3 * row * _cols * _preview_width * _preview_height;
    }

    for (size_t i=0; i<_preview_height; ++i) {
        memcpy(_buff + pos, buff + (3 * i * _preview_width), 3 * _preview_width);
        pos += 3 * _cols * _preview_width;
    }

    ++_added_number_of_previews;

    return true;
}
