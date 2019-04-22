#include "preview_map_builder.h"

#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

preview_map_builder::preview_map_builder(const preview_map_format& main_format,
                                         const std::vector<preview_map_format>& sub_formats,
                                         int64_t flush_duration_msecs) noexcept :
    _flush_duration_msecs(flush_duration_msecs),
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
    for (auto& sub_format : _sub_formats) {
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

    preview_item_info& item_info = map->items_info[item_number];

    if (item_info.empty || abs(item_info.offset_msecs) > abs(item_offset_msecs)) {
        if (map->map->insert(item_number, data, data_size)) {
            item_info.empty = false;
            item_info.offset_msecs = item_offset_msecs;

            ++map->item_counter;

            const int64_t current_map_duration_msecs =
                    static_cast<int64_t>(map->item_counter) * format.format.item_duration_msecs;

            bool mapIsBuilded = map->item_counter >= format.format.items;
            bool mapIsReadyToFlush = false;
            if (current_map_duration_msecs/_flush_duration_msecs
                    > static_cast<int64_t>(map->flush_counter)) {
                mapIsReadyToFlush = true;
            }
            bool mapIsReadyToSave = mapIsBuilded || mapIsReadyToFlush;

            if (mapIsReadyToSave && SaveMapHandler) {
                int64_t map_start_ut_msecs = static_cast<int64_t>(
                            (items - item_number)) * format.format.item_duration_msecs;

                SaveMapHandler(map_start_ut_msecs,
                               format.format,
                               map->map,
                               map->items_info);

                ++map->flush_counter;

                if (mapIsBuilded) {
                    format.maps.erase(search);
                }
            }
        }
    }

    // TODO: return result
}
