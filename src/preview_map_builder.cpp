#include "preview_map_builder.h"

#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

preview_map_builder::preview_map_builder(task tsk) noexcept
    : _task(tsk)
{
    // TODO: check task
}

preview_map_builder::error_type
preview_map_builder::insert(int64_t start_ut_msecs,
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
    if (duration_msecs > _task.format.item_duration_msecs) {
        std::cout << "warning: duration " << duration_msecs
                  << " more than " << _task.format.item_duration_msecs << std::endl;
    }
    // TODO: check width and height with main_format

    map_format& main_format = _task.format;

    private_insert(start_ut_msecs, data, data_size, main_format);

    const size_t items = static_cast<size_t>(
                start_ut_msecs / main_format.item_duration_msecs);
    for (auto& sub_format : _task.sub_formats) {
        size_t ratio = static_cast<size_t>(
                    sub_format.item_duration_msecs / main_format.item_duration_msecs);
        if (items % ratio != 0) {
            continue;
        }

        const char* sub_data = nullptr;
        size_t sub_data_size = 0;

        // TODO: one time scaling for the same map size
        if (sub_format.item_width_px < width_px
                || sub_format.item_height_px < height_px) {
            cv::Mat in(static_cast<int>(height_px),
                       static_cast<int>(width_px),
                       CV_8UC3,
                       const_cast<char*>(data));
            cv::Mat out;

            cv::Size size;
            size.width = static_cast<int>(sub_format.item_width_px);
            size.height = static_cast<int>(sub_format.item_height_px);
            // TODO: exception?
            cv::resize(in, out, size);

            sub_data = reinterpret_cast<char*>(out.data);
            sub_data_size = out.total() * out.elemSize();
        } else {
            sub_data = data;
            sub_data_size = data_size;
        }

        private_insert(start_ut_msecs, sub_data, sub_data_size, sub_format);
    }

    // TODO: res
    return error_type::none_error;
}

void preview_map_builder::private_insert(int64_t start_ut_msecs,
                                         const char* data,
                                         size_t data_size,
                                         map_format& format) noexcept
{
    const size_t items = static_cast<size_t>(
                start_ut_msecs / format.item_duration_msecs);
    size_t map_number = items / format.items;
    size_t item_number = items % format.items;

    const int64_t item_offset_msecs =
            start_ut_msecs % format.item_duration_msecs;
    if (item_offset_msecs > (format.item_duration_msecs / 2)) {
        ++item_number;
        if (item_number + 1 > format.items) {
            item_number = 0;
            ++map_number;
        }
    }

    std::shared_ptr<preview_map> map;
    auto search = format.maps.find(map_number);
    if (search != format.maps.end()) {
        map = search->second;
    } else {
        map = std::make_shared<preview_map>(format.rows,
                                            format.cols,
                                            format.item_width_px,
                                            format.item_height_px);
        format.maps.insert({map_number, map});
    }

    // TODO: check preview offset if this the item exists
    if (map->insert_preview(item_number, data, data_size)) {
        if (map->is_full() && MapBuildedHandler) {
            // TODO: calculate
            int64_t start_ut_msecs = 0;
            MapBuildedHandler(start_ut_msecs, format, map);
            // TODO: release
        }
    }

    // TODO: return result
}
