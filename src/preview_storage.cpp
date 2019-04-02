#include "preview_storage.h"

#include <ctime>
#include <cassert>
#include <iostream>

preview_storage::preview_storage() :
    _current_day_number(std::time(nullptr) / 86400)
{
    static_assert(_number_of_msecs_per_day % _map_duration_msecs == 0, "A number of matrix per day must be an integer value");
    static_assert(_map_item_duration_msecs % 2 == 0, "A map item duration must be an even number");

    // TODO: Does exist another way to do it?
    _preview_maps.reserve(_number_of_maps_per_day);
    for (int i=0; i<_number_of_maps_per_day; ++i) {
        _preview_maps.push_back(nullptr);
    }
}

bool preview_storage::add_preview(int64_t start_ut_msecs,
                                  int64_t duration_msecs)
{
    if (start_ut_msecs < 0) {
        std::cerr << "invalid start_timestamp_msecs " << start_ut_msecs;
        return false;
    }
    if (duration_msecs < 0) {
        std::cerr << "invalid duration " << duration_msecs;
        return false;
    }
    // TODO: don't ignore this preview
    if (duration_msecs >= _map_item_duration_msecs) {
        std::cerr << "duration " << duration_msecs << " more than " << _map_item_duration_msecs;
        return false;
    }

    const size_t day_number = static_cast<size_t>(start_ut_msecs / _number_of_msecs_per_day);
    const size_t day_start_ut_msecs = start_ut_msecs % _number_of_msecs_per_day;
    const size_t map_number = day_start_ut_msecs / _map_duration_msecs;

    assert(map_number >= 0 && map_number < _number_of_maps_per_day);

    // TODO: allocate one time for a day
    auto& map = _preview_maps[map_number];
    if (map == nullptr) {
        map = std::make_shared<preview_map>(_number_of_rows,
                                            _number_of_columns,
                                            _map_item_duration_msecs);
        assert(_preview_maps.at(map_number) != nullptr);
    }

    // TODO: before calculate it and next define a map number, becuase it
    // might be a next map that one below
    const size_t map_start_ut_msecs =
            day_start_ut_msecs % _map_duration_msecs;

    const size_t map_item_start_ut_msecs =
            map_start_ut_msecs % _map_item_duration_msecs;

    size_t item_number = static_cast<size_t>(
                map_start_ut_msecs / _map_item_duration_msecs);

    if (map_item_start_ut_msecs > (_map_item_duration_msecs / 2)) {
        // TODO: it might be a next map
        ++item_number;
    }

    // TODO: move to map_preview
    const size_t row_number = item_number / _number_of_columns;
    const size_t column_number = item_number % _number_of_columns;

    return map->add_preview(row_number, column_number);
}
