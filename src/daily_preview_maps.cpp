#include "daily_preview_maps.h"

daily_preview_maps::daily_preview_maps(int64_t one_map_duration_msecs) :
    _one_map_duration_msecs(one_map_duration_msecs)
{
}

bool daily_preview_maps::add_preview(int64_t start_timestamp_msecs,
                                     int64_t duration_msecs)
{
    const std::size_t matrix_row_count = 4;
    const std::size_t matrix_column_count = 4;


    const std::size_t map_number = start_timestamp_msecs / _one_map_duration_msecs;
    const std::size_t cell_number = (start_timestamp_msecs % _one_map_duration_msecs) / duration_msecs;
    std::size_t row_index = cell_number / matrix_column_count;
    if (cell_number % matrix_column_count) {
        ++row_index;
    }
    const std::size_t column_index = matrix_column_count - (cell_number % matrix_column_count);

    return false;
}
