#include "preview_map.h"

#include <cstring>
#include <iostream>

preview_map::preview_map(size_t rows,
                         size_t cols,
                         size_t item_width_px,
                         size_t item_height_px) noexcept :
    _rows(rows),
    _cols(cols),
    _items(_rows*_cols),
    _item_width_px(item_width_px),
    _item_height_px(item_height_px),
    _item_size(item_width_px * item_height_px * 3),
    _map_size(_item_size * _items)
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
    return (_added_number_of_previews == _items);
}

const char *preview_map::data() const noexcept
{
    return _buff;
}

size_t preview_map::size() const noexcept
{
    return _map_size;
}

size_t preview_map::width_px() const
{
    return _cols * _item_width_px;
}

size_t preview_map::height_px() const
{
    return _rows * _item_height_px;
}

bool preview_map::insert(size_t number,
                         const char* buff,
                         std::size_t size) noexcept
{
    if (size != _item_size) {
        std::cerr << "size must be " << _item_size
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

    size_t pos = (3 * col * _item_width_px);
    if (row > 0) {
        pos += 3 * row * _cols * _item_width_px * _item_height_px;
    }

    for (size_t i=0; i<_item_height_px; ++i) {
        memcpy(_buff + pos, buff + (3 * i * _item_width_px), 3 * _item_width_px);
        pos += 3 * _cols * _item_width_px;
    }

    ++_added_number_of_previews;

    return true;
}
