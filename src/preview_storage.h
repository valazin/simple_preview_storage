#ifndef PREVIEW_STORAGE_H
#define PREVIEW_STORAGE_H

#include <cstdint>
#include <vector>
#include <memory>

#include "http/buffer.h"

#include "preview_map.h"

class preview_storage
{
public:
    explicit preview_storage(const std::string& dir_path) noexcept;

    bool add_preview(const std::string& id,
                     int64_t start_ut_msecs,
                     int64_t duration_msecs,
                     size_t width,
                     size_t height,
                     std::shared_ptr<http::buffer> buff) noexcept;

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
    bool save_preview_map(const std::shared_ptr<preview_map>& map,
                          const std::string& file_path) const noexcept;

    bool create_preview_dir(const std::string& id,
                            size_t day_number) const noexcept;

    std::string preview_dir_path(const std::string& id,
                                 size_t day_number) const noexcept;
    std::string preview_file_name(size_t map_number) const noexcept;
    std::string preview_file_path(const std::string& id,
                                  size_t day_number,
                                  size_t map_number) const noexcept;

private:
    static constexpr size_t _rows = 5;
    static constexpr size_t _cols = 6;
    static constexpr int64_t _number_of_previews_per_map = _rows * _cols;
    static constexpr int64_t _preview_duration_msecs = 10000;
    static constexpr int64_t _map_duration_msecs =
            _number_of_previews_per_map * _preview_duration_msecs;
    static constexpr int64_t _number_of_msecs_per_day = 60 * 60 * 24 * 1000;
    static constexpr int64_t _number_of_maps_per_day =
            _number_of_msecs_per_day / _map_duration_msecs;

    const std::string _work_dir_path;

    const size_t c_center_day_number = 0;
    std::vector<std::shared_ptr<preview_group>> _left_daily_preview_groups;
    std::vector<std::shared_ptr<preview_group>> _right_daily_preview_groups;
};

#endif // PREVIEW_STORAGE_H
