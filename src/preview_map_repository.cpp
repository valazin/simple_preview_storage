#include "preview_map_repository.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

#include <opencv2/imgcodecs.hpp>

#include "filesystem.h"

preview_map_repository::preview_map_repository(const std::string& dir_path) noexcept :
    _dir_path(dir_path)
{
    // TODO: create dir
}

preview_map_repository::error_type
preview_map_repository::save(const std::string& id,
                             int64_t start_ut_msecs,
                             const preview_map_format& format,
                             std::shared_ptr<preview_map> map,
                             const std::vector<preview_item_info>& items_info) noexcept
{
    const file_info info = preview_file_info(id, start_ut_msecs, format);

    std::string dir_path = _dir_path;
    for (auto& dir : info.relative_dir_path) {
        dir_path += "/" + dir;
        if (!filesystem::dir_is_exist(dir_path)
                && !filesystem::create_directory(dir_path)) {
            return error_type::file_creating_error;
        }
    }

    const std::string file_path = dir_path + "/" + info.file_name;
    const std::string meta_file_path = file_path + "_meta";

    error_type error = save_preview_map_to_file(map, file_path);
    if (error == error_type::none_error) {
        error = save_preview_offsets_to_file(items_info, meta_file_path);
    }

    return error;
}

preview_map_repository::error_type
preview_map_repository::save_preview_map_to_file(const std::shared_ptr<preview_map> &map,
                                                 const std::string &file_path) noexcept
{
    std::vector<uchar> out_buff(map->size(), 0);
    cv::Mat in_mat(static_cast<int>(map->height_px()),
                   static_cast<int>(map->width_px()),
                   CV_8UC3,
                   const_cast<char*>(map->data()));

    const std::vector<int> params {
        cv::ImwriteFlags::IMWRITE_JPEG_QUALITY, 65
    };

    // TODO: exeption?
    bool res = cv::imencode(".jpg", in_mat, out_buff, params);

    if (res) {
        return save_to_file(out_buff.data(),out_buff.size(), file_path);
    } else {
        return error_type::image_encoding_error;
    }
}

preview_map_repository::error_type
preview_map_repository::save_preview_offsets_to_file(const std::vector<preview_item_info>& items_info,
                                                     const std::string& file_path) noexcept
{
    std::string text;
    for (size_t i=0; i<items_info.size(); ++i) {
        const preview_item_info& info = items_info[i];
        if (!info.empty) {
            text += std::to_string(info.offset_msecs/1000);
        } else {
            text += "null";
        }
        if (i != items_info.size() - 1) {
            text += ";";
        }
    }
    return save_to_file(text.data(), text.size(), file_path);
}

preview_map_repository::file_info
preview_map_repository::preview_file_info(const std::string& id,
                                          int64_t start_ut_msecs,
                                          const preview_map_format& format) noexcept
{
    // TODO: move to datetime utility
    std::time_t start_ut_secs = start_ut_msecs / 1000;
    struct std::tm* utc_date = gmtime(&start_ut_secs);

    const int day = utc_date->tm_mday;
    const int month = utc_date->tm_mon + 1;
    const int year = utc_date->tm_year + 1900;

    const std::string dir1 = id;

    std::string dir2 = std::to_string(year)+ "-";
    if (month < 9) {
        dir2 += "0";
    }
    dir2 +=  std::to_string(month) + "-";
    if (day < 9) {
        dir2 += "0";
    }
    dir2 += std::to_string(day);

    const std::vector<std::string> relative_dir_path =  {dir1, dir2};

    const size_t map_number = (static_cast<size_t>(start_ut_msecs) % 86400000)
            / static_cast<size_t>(format.map_duration_msecs);

    std::string file_name = std::to_string(map_number)
            + "_"
            + std::to_string(format.item_duration_msecs)
            + "msecs";

    return {relative_dir_path, file_name};
}

preview_map_repository::error_type
preview_map_repository::save_to_file(const void *data,
                                     size_t data_size,
                                     const std::string& file_path) noexcept
{
    // TODO: move to filesystem utility
    int fd = open(file_path.data(),
                  O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd != -1) {
        ssize_t written_size = write(fd, data, data_size);
        if (written_size != -1) {
            // TODO: if a written size less than a preview size
            close(fd);
            return error_type::none_error;
        } else {
            perror("write map to file");
            return error_type::file_creating_error;
        }
    } else {
        perror("open file");
        return error_type::file_creating_error;
    }
}
