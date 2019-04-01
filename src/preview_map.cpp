#include "preview_map.h"

preview_map::preview_map(int row_number,
                         int column_number,
                         int64_t item_duration_msecs) :
    _row_number(row_number),
    _column_number(column_number),
    _item_duration_msecs(item_duration_msecs),
    _total_duration_msecs(_row_number * _column_number * _item_duration_msecs)
{
}

bool preview_map::add_preview(int row_index, int column_index)
{
}
