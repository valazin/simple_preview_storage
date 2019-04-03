#include "preview_storage.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <ctime>
#include <cassert>
#include <iostream>

#include "filesystem.h"

preview_storage::preview_storage(const std::string &dir_path) noexcept :
    c_dir_path(dir_path),
    c_center_day_number(static_cast<size_t>(std::time(nullptr) / 86400) - 1),
    _left_daily_preview_groups(500, nullptr),
    _right_daily_preview_groups(500, nullptr)
{
    static_assert(c_number_of_msecs_per_day % c_map_duration_msecs == 0,
                  "A number of matrix per day must be an integer value");
    static_assert(c_preview_duration_msecs % 2 == 0,
                  "A map preview duration must be an even number");
}

bool preview_storage::add_preview(int64_t start_ut_msecs,
                                  int64_t duration_msecs,
                                  size_t width,
                                  size_t height,
                                  char* buff,
                                  size_t size) noexcept
{
    if (start_ut_msecs < 0) {
        std::cerr << "invalid start_timestamp_msecs " << start_ut_msecs << std::endl;
        return false;
    }
    if (duration_msecs < 0) {
        std::cerr << "invalid duration " << duration_msecs << std::endl;
        return false;
    }
    if (duration_msecs >= c_preview_duration_msecs) {
        std::cout << "warning: duration " << duration_msecs
                  << " more than " << c_preview_duration_msecs << std::endl;
    }

    // calculate preview coordinates
    const size_t day_start_ut_msecs =
            start_ut_msecs % c_number_of_msecs_per_day;
    const size_t map_start_ut_msecs =
            day_start_ut_msecs % c_map_duration_msecs;
    const size_t map_preview_start_ut_msecs =
            map_start_ut_msecs % c_preview_duration_msecs;

    size_t day_number = static_cast<size_t>(
                start_ut_msecs / c_number_of_msecs_per_day);
    size_t map_number = day_start_ut_msecs / c_map_duration_msecs;
    size_t preview_number = static_cast<size_t>(
                map_start_ut_msecs / c_preview_duration_msecs);

    if (map_preview_start_ut_msecs > (c_preview_duration_msecs / 2)) {
        ++preview_number;
    }
    if (preview_number + 1 > c_number_of_previews_per_map) {
        ++map_number;
        preview_number = 0;
        if (map_number + 1 > c_number_of_maps_per_day) {
            ++day_number;
            map_number = 0;
        }
    }

    std::cout << day_number << " " << map_number << " " << preview_number << std::endl;

    assert(map_number >= 0 && map_number < c_number_of_maps_per_day);

    // select the day
    std::shared_ptr<preview_group> daily_group;
    int64_t distance = static_cast<int64_t>(day_number - c_center_day_number);
    if (distance >= 0) {
        std::cout << "use right daily group" << std::endl;

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
            daily_group = std::make_shared<preview_group>(c_number_of_maps_per_day);
            _right_daily_preview_groups[index] = daily_group;
        }
    } else {
        std::cout << "use left daily group" << std::endl;

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
            daily_group = std::make_shared<preview_group>(c_number_of_maps_per_day);
            _left_daily_preview_groups[index] = daily_group;
        }
    }
    assert(daily_group != nullptr);

    // select the map inside the day
    auto& map = daily_group->preview_maps[map_number];
    if (map == nullptr) {
        map = std::make_shared<preview_map>(c_number_of_rows,
                                            c_number_of_columns,
                                            width,
                                            height);
        assert(daily_group->preview_maps.at(map_number) != nullptr);
    }

    // TODO: + metainfo
    // TODO: use weight
    if (map->insert_preview(preview_number, buff, size)) {
        if (map->is_full()) {
            std::cout << "map " << map_number << " is full" << std::endl;
            const std::string preview_dir_path =
                    c_dir_path + "/" + std::to_string(day_number);
            if (filesystem::dir_is_exist(preview_dir_path)
                    || filesystem::create_directory(preview_dir_path)) {
                const std::string preview_file_path =
                        preview_dir_path+ "/" + std::to_string(map_number);

                std::cout << "saving map " << map_number
                          << " to file " << preview_file_path << std::endl;
                int fd = open(preview_file_path.data(),
                              O_WRONLY | O_CREAT | O_TRUNC,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

                if (fd != -1) {
                    ssize_t written_size = write(fd, map->data(), map->size());
                    if (written_size != -1) {
                        std::cout << "success save map " << map_number
                                  << " to file " << preview_file_path << std::endl;

                        // TODO: if a written size less than a preview size

                        daily_group->preview_maps[map_number] = nullptr;
                    } else {
                        std::cerr << "error save map " << map_number
                                  << " to file " << preview_file_path << std::endl;
                        perror("write map to file");
                    }
                }
            } else {
                std::cerr << "couldn't save map " << map_number
                          << " create dir error " << preview_dir_path << std::endl;
            }
        }
        return true;
    } else {
        return false;
    }
}
