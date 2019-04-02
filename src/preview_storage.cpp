#include "preview_storage.h"

#include <ctime>
#include <cassert>
#include <iostream>

preview_storage::preview_storage() :
    _center_day_number(static_cast<size_t>(std::time(nullptr) / 86400) - 1),
    _left_daily_preview_groups(500, nullptr),
    _right_daily_preview_groups(500, nullptr)
{
    static_assert(_number_of_msecs_per_day % _map_duration_msecs == 0,
                  "A number of matrix per day must be an integer value");
    static_assert(_map_item_duration_msecs % 2 == 0,
                  "A map item duration must be an even number");
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

    const size_t day_start_ut_msecs =
            start_ut_msecs % _number_of_msecs_per_day;
    const size_t map_start_ut_msecs =
            day_start_ut_msecs % _map_duration_msecs;
    const size_t map_item_start_ut_msecs =
            map_start_ut_msecs % _map_item_duration_msecs;

    size_t day_number = static_cast<size_t>(
                start_ut_msecs / _number_of_msecs_per_day);
    size_t map_number = day_start_ut_msecs / _map_duration_msecs;
    size_t map_item_number = static_cast<size_t>(
                map_start_ut_msecs / _map_item_duration_msecs);

    if (map_item_start_ut_msecs > (_map_item_duration_msecs / 2)) {
        ++map_item_number;
    }
    if (map_item_number + 1 > _number_of_items_per_map) {
        ++map_number;
        map_item_number = 0;
        if (map_number + 1 > _number_of_maps_per_day) {
            ++day_number;
            map_number = 0;
        }
    }

    std::cout << day_number << " " << map_number << " " << map_item_number << std::endl;

    assert(map_number >= 0 && map_number < _number_of_maps_per_day);

    // select the day
    std::shared_ptr<preview_group> daily_group;
    int64_t distance = static_cast<int64_t>(day_number - _center_day_number);
    if (distance >= 0) {
        std::cout << "use right group" << std::endl;

        const size_t index = static_cast<size_t>(distance);
        if (index >= _right_daily_preview_groups.size()) {
            const size_t count = index - _right_daily_preview_groups.size() + 1;
            std::cout << "start to add " << count << std::endl;
            for (size_t i=0; i<count; ++i) {
                _right_daily_preview_groups.push_back(nullptr);
            }
        }
        assert(index >= 0 && index < _right_daily_preview_groups.size());

        daily_group = _right_daily_preview_groups.at(index);
        if (daily_group == nullptr) {
            daily_group = std::make_shared<preview_group>(_number_of_maps_per_day);
            _right_daily_preview_groups[index] = daily_group;
        }
    } else {
        std::cout << "use left group" << std::endl;

        const size_t index = static_cast<size_t>((-1*distance) - 1);
        if (index >= _left_daily_preview_groups.size()) {
            const size_t count = index - _left_daily_preview_groups.size() + 1;
            std::cout << "start to add " << count << std::endl;
            for (size_t i=0; i<count; ++i) {
                _left_daily_preview_groups.push_back(nullptr);
            }
        }
        assert(index >= 0 && index < _left_daily_preview_groups.size());

        daily_group = _left_daily_preview_groups.at(index);
        if (daily_group == nullptr) {
            daily_group = std::make_shared<preview_group>(_number_of_maps_per_day);
            _left_daily_preview_groups[index] = daily_group;
        }
    }
    assert(daily_group != nullptr);

    // select the map inside the day
    auto& map = daily_group->preview_maps[map_number];
    if (map == nullptr) {
        map = std::make_shared<preview_map>(_number_of_rows,
                                            _number_of_columns,
                                            _map_item_duration_msecs);
        assert(daily_group->preview_maps.at(map_number) != nullptr);
    }

    // TODO: move to map_preview
    const size_t row_number = map_item_number / _number_of_columns;
    const size_t column_number = map_item_number % _number_of_columns;

    // TODO: + metainfo
    return map->add_preview(row_number, column_number);
}
