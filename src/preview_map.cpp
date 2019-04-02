#include "preview_map.h"

#include <cstring>
#include <iostream>

preview_map::preview_map(size_t number_of_rows,
                         size_t number_of_columns,
                         size_t item_width,
                         size_t item_height) :
    _number_of_rows(number_of_rows),
    _number_of_columns(number_of_columns),
    _item_width(item_width),
    _item_height(item_height),
    _item_size(item_width * item_height * 3)
{
}

bool preview_map::add_preview(size_t item_number, char* buff, size_t size)
{
    if (size != _item_size) {
        std::cerr << "size must be " << _item_size << " but it's " << size << std::endl;
        return false;
    }

    const size_t row_number = item_number / _number_of_columns;
    const size_t column_number = item_number % _number_of_columns;

    if (!_buff_was_allocated) {
        _buff = new char[_number_of_rows * _number_of_columns * _item_size];
        _buff_was_allocated = true;
    }

    memcpy(_buff + item_number*_item_size, buff, size);

    return false;
}
