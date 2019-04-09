#include "preview_map_builder.h"

#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

preview_map_builder::preview_map_builder(const preview_map_format& main_format,
                                         const std::vector<preview_map_format>& sub_formats) noexcept :
    _main_format{main_format, {}}
{
    // TODO: validate sub_formats

    std::vector<private_format> sub;
    sub.reserve(sub_formats.size());
    for (auto& format : sub_formats) {
        sub.push_back({format, {}});
    }
    _sub_formats = sub;
}

preview_map_builder::error_type
preview_map_builder::add_preview(int64_t start_ut_msecs,
                            int64_t duration_msecs,
                            size_t width_px,
                            size_t height_px,
                            const char *data,
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

    const size_t items = static_cast<size_t>(
                start_ut_msecs / main_format.item_duration_msecs);
    for (auto& sub_format : _sub_formats) {
        size_t ratio = static_cast<size_t>(
                    sub_format.format.item_duration_msecs / main_format.item_duration_msecs);
        if (items % ratio != 0) {
            continue;
        }

        const char* sub_data = nullptr;
        size_t sub_data_size = 0;

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

            sub_data = reinterpret_cast<char*>(out.data);
            sub_data_size = out.total() * out.elemSize();
        } else {
            sub_data = data;
            sub_data_size = data_size;
        }

        insert(start_ut_msecs, sub_data, sub_data_size, sub_format);
    }

    // TODO: res
    return error_type::none_error;
}

void preview_map_builder::insert(int64_t start_ut_msecs,
                                 const char* data,
                                 size_t data_size,
                                 private_format& format) noexcept
{
    const size_t items = static_cast<size_t>(
                start_ut_msecs / format.format.item_duration_msecs);
    size_t map_number = items / format.format.items;
    size_t item_number = items % format.format.items;

    const int64_t item_offset_msecs =
            start_ut_msecs % format.format.item_duration_msecs;
    if (item_offset_msecs > (format.format.item_duration_msecs / 2)) {
        ++item_number;
        if (item_number + 1 > format.format.items) {
            item_number = 0;
            ++map_number;
        }
    }

    // TODO: try load from repository

    std::shared_ptr<private_map> map;
    auto search = format.maps.find(map_number);
    if (search != format.maps.end()) {
        map = search->second;
    } else {
        map = std::make_shared<private_map>();
        map->map = std::make_shared<preview_map>(format.format.rows,
                                                 format.format.cols,
                                                 format.format.item_width_px,
                                                 format.format.item_height_px);
        map->items_offset_msecs = std::vector<int64_t>(format.format.items, -1);
        format.maps.insert({map_number, map});
    }

    // TODO: sometimes flush to disk

    if (map->items_offset_msecs.at(item_number) == -1 ||
            map->items_offset_msecs.at(item_number) > item_offset_msecs) {
        if (map->map->insert_preview(item_number, data, data_size)) {
            map->items_offset_msecs[item_number] = item_offset_msecs;

            if (map->map->is_full() && MapBuildedHandler) {
                int64_t start_ut_msecs = static_cast<int64_t>(
                            (items - item_number)) * format.format.item_duration_msecs;

                MapBuildedHandler(start_ut_msecs,
                                  format.format,
                                  map->map,
                                  map->items_offset_msecs);

                format.maps.erase(search);
            }
        }
    } else {
        // TODO: increase count added items. maybe move this counter to private_map
    }

    // TODO: return result
}
