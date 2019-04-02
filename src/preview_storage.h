#ifndef PREVIEW_STORAGE_H
#define PREVIEW_STORAGE_H

#include <cstdint>
#include <vector>
#include <memory>

#include "preview_map.h"

class preview_storage
{
public:
    preview_storage();

    bool add_preview(int64_t start_ut_msecs, int64_t duration_msecs);

private:

private:
    static constexpr size_t _number_of_rows = 5;
    static constexpr size_t _number_of_columns = 6;
    static constexpr int64_t _map_item_duration_msecs = 10000;
    static constexpr int64_t _map_duration_msecs = _number_of_rows * _number_of_columns * _map_item_duration_msecs;
    static constexpr int64_t _number_of_msecs_per_day = 60 * 60 * 24 * 1000;
    static constexpr int64_t _number_of_maps_per_day = _number_of_msecs_per_day / _map_duration_msecs;

    int64_t _current_day_number = 0;

    std::vector<std::shared_ptr<preview_map>> _preview_maps;
};

#endif // PREVIEW_STORAGE_H
