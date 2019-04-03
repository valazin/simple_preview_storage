#ifndef PREVIEW_STORAGE_H
#define PREVIEW_STORAGE_H

#include <cstdint>
#include <vector>
#include <memory>

#include "preview_map.h"

class preview_storage
{
public:
    explicit preview_storage(const std::string& dir_path);

    bool add_preview(int64_t start_ut_msecs,
                     int64_t duration_msecs,
                     size_t width,
                     size_t height,
                     char* buff,
                     size_t size) noexcept;

private:
    struct preview_group
    {
        preview_group(size_t size) :
            preview_maps(size, nullptr)
        {
        }

        // TODO: test without shared_ptr
        std::vector<std::shared_ptr<preview_map>> preview_maps;
    };

private:
    static constexpr size_t c_number_of_rows = 5;
    static constexpr size_t c_number_of_columns = 6;
    static constexpr int64_t c_number_of_items_per_map =
            c_number_of_rows * c_number_of_columns;
    static constexpr int64_t c_map_item_duration_msecs = 10000;
    static constexpr int64_t c_map_duration_msecs =
            c_number_of_items_per_map * c_map_item_duration_msecs;
    static constexpr int64_t c_number_of_msecs_per_day = 60 * 60 * 24 * 1000;
    static constexpr int64_t c_number_of_maps_per_day =
            c_number_of_msecs_per_day / c_map_duration_msecs;

    const std::string c_dir_path;

    const size_t c_center_day_number = 0;
    std::vector<std::shared_ptr<preview_group>> _left_daily_preview_groups;
    std::vector<std::shared_ptr<preview_group>> _right_daily_preview_groups;
};

#endif // PREVIEW_STORAGE_H
