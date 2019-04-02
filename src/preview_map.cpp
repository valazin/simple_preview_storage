#include "preview_map.h"

#include <iostream>

preview_map::preview_map(size_t number_of_rows,
                         size_t number_of_columns,
                         int64_t item_duration_msecs) :
    _number_of_rows(number_of_rows),
    _number_of_columns(number_of_columns),
    _item_duration_msecs(item_duration_msecs)
{
}

bool preview_map::add_preview(size_t row_number, size_t column_number)
{
    std::cout << row_number << " " << column_number << std::endl;
    return false;
}
