#include "preview_map_builder.h"

#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "utility/datetime.h"

preview_map_builder::preview_map_builder(const preview_map_format& main_format,
        const std::vector<preview_map_format>& sub_formats,
        int64_t flush_timeout_msecs) noexcept :
    _flush_timeout_msecs(flush_timeout_msecs),
    _main_format{main_format, {}}
{
    // TODO: validate sub_formats

    _sub_formats.reserve(sub_formats.size());
    for (auto& format : sub_formats) {
        _sub_formats.push_back({format, {}});
    }
}

preview_map_builder::error_type
preview_map_builder::add_preview(int64_t start_ut_msecs,
                                 int64_t duration_msecs,
                                 size_t width_px,
                                 size_t height_px,
                                 const char* data,
                                 size_t data_size) noexcept
{
    if (start_ut_msecs < 0) {
        std::cerr << "invalid start_timestamp_msecs " << start_ut_msecs << std::endl;
        return error_type::invalid_arguments;
    }
    if (duration_msecs < 0) {
        std::cerr << "invalid duration " << duration_msecs << std::endl;
        return error_type::invalid_arguments;
    }
    if (width_px <=0 || height_px <=0) {
        std::cerr << "invalid size " << width_px << " " << height_px << std::endl;
        return error_type::invalid_arguments;
    }
    if (data == nullptr) {
        std::cerr << "invalid data ptr " << std::endl;
        return error_type::invalid_arguments;
    }
    if (data_size < width_px*height_px) {
        std::cerr << "invalid data size " << data_size << std::endl;
        return error_type::invalid_arguments;
    }
    if (duration_msecs > _main_format.format.item_duration_msecs) {
        std::cout << "warning: duration " << duration_msecs
                  << " more than " << _main_format.format.item_duration_msecs << std::endl;
    }
    // TODO: check width and height with main_format

    preview_map_format& main_format = _main_format.format;

    insert(start_ut_msecs, data, data_size, _main_format);

    const size_t items = static_cast<size_t>(start_ut_msecs
                                             / main_format.item_duration_msecs);
    for (auto&& sub_format : _sub_formats) {
        size_t ratio = static_cast<size_t>(sub_format.format.item_duration_msecs
                                           / main_format.item_duration_msecs);
        if (items % ratio != 0) {
            continue;
        }

        // TODO: one time scaling for the same map size
        if (sub_format.format.item_width_px < width_px
                || sub_format.format.item_height_px < height_px) {
            cv::Mat in(static_cast<int>(height_px),
                       static_cast<int>(width_px),
                       CV_8UC3,
                       const_cast<char*>(data));
            cv::Mat out;

            cv::Size size;
            size.width = static_cast<int>(sub_format.format.item_width_px);
            size.height = static_cast<int>(sub_format.format.item_height_px);
            // TODO: exception?
            cv::resize(in, out, size);

            const char* resize_data = reinterpret_cast<char*>(out.data);
            const size_t resize_data_size = out.total() * out.elemSize();
            insert(start_ut_msecs, resize_data, resize_data_size, sub_format);
        } else {
            insert(start_ut_msecs, data, data_size, sub_format);
        }
    }

    // TODO: res
    return error_type::none_error;
}

bool preview_map_builder::empty() const noexcept
{
    if (_main_format.maps.empty()) {
        for (auto&& sub_format : _sub_formats) {
            if (!sub_format.maps.empty()) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}

size_t preview_map_builder::count() const noexcept
{
    size_t res = _main_format.maps.size();
    for (auto&& sub_format : _sub_formats) {
        res += sub_format.maps.size();
    }
    return res;
}

std::tuple<size_t, preview_map_builder::error_type>
preview_map_builder::release_maps(int64_t unmodified_secs) noexcept
{
    size_t res = 0;

    const int64_t now = datetime::now_ut_msecs();

    auto release = [this, now, unmodified_secs] (private_format& format) -> size_t {
        size_t res = 0;

        auto i = format.maps.begin();
        while (i != format.maps.end()) {
            auto map = i->second;
            if (now - map->last_modyfied_ut >= unmodified_secs) {
                if (map->has_unsave_changes) {
                    SaveMapHandler(map->start_ut_msecs,
                                   format.format,
                                   map->map,
                                   map->items_info);
                }
                i = format.maps.erase(i);
                ++res;
            } else {
                ++i;
            }
        }

        return res;
    };

    res += release(_main_format);
    for (auto&& sub_format : _sub_formats) {
        res += release(sub_format);
    }

    return {res, error_type::none_error};
}

void preview_map_builder::insert(int64_t start_ut_msecs,
                                 const char* data,
                                 size_t data_size,
                                 private_format& format) noexcept
{
    // a number of previews followed by the item number
    size_t items = static_cast<size_t>(
                start_ut_msecs / format.format.item_duration_msecs);
    size_t map_number = items / format.format.items;
    size_t item_number = items % format.format.items;

    int64_t item_offset_msecs =
            start_ut_msecs % format.format.item_duration_msecs;
    if (item_offset_msecs > (format.format.item_duration_msecs / 2)) {
        ++items;
        ++item_number;
        if (item_number + 1 > format.format.items) {
            item_number = 0;
            ++map_number;
        }
        item_offset_msecs -= format.format.item_duration_msecs;
    }

    // TODO: try load from repository

    std::shared_ptr<private_map> map;
    auto search = format.maps.find(map_number);
    if (search != format.maps.end()) {
        map = search->second;
    } else {
        map = std::make_shared<private_map>();
        map->start_ut_msecs = static_cast<int64_t>((items - item_number))
                * format.format.item_duration_msecs;

        map->map = std::make_shared<preview_map>(format.format.rows,
                                                 format.format.cols,
                                                 format.format.item_width_px,
                                                 format.format.item_height_px);

        preview_item_info default_info;
        default_info.empty = true;
        default_info.offset_msecs = 0;
        map->items_info = std::vector<preview_item_info>{format.format.items, default_info};

        format.maps.insert({map_number, map});
    }

    map->last_modyfied_ut = datetime::now_ut_msecs();

    preview_item_info& item_info = map->items_info[item_number];

    if (item_info.empty || abs(item_info.offset_msecs) > abs(item_offset_msecs)) {
        if (map->map->insert(item_number, data, data_size)) {
            item_info.empty = false;
            item_info.offset_msecs = item_offset_msecs;

            ++map->item_counter;

            map->has_unsave_changes = true;

            const bool is_builded = map_is_builded(map, format);
            const bool is_ready_to_flush = map_is_ready_to_flush_by_timeout(map, format);
            const bool is_ready_to_save = is_builded || is_ready_to_flush;

            if (is_ready_to_save && SaveMapHandler) {
                SaveMapHandler(map->start_ut_msecs,
                               format.format,
                               map->map,
                               map->items_info);

                map->has_unsave_changes = false;

                if (!is_builded) {
                    ++map->flush_counter;
                } else {
                    format.maps.erase(search);
                }
            }
        }
    }

    // TODO: return result
}

bool preview_map_builder::map_is_builded(
        std::shared_ptr<private_map> map,
        const private_format &format) const noexcept
{
    return map->item_counter >= format.format.items;
}

bool preview_map_builder::map_is_ready_to_flush_by_timeout(
        std::shared_ptr<private_map> map,
        const private_format &format) const noexcept
{
    const int64_t current_map_duration_msecs =
            static_cast<int64_t>(map->item_counter)
            * format.format.item_duration_msecs;

    bool res = false;
    if (current_map_duration_msecs/_flush_timeout_msecs
            > static_cast<int64_t>(map->flush_counter)) {
        res = true;
    }

    return res;
}
