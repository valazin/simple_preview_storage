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
                             std::shared_ptr<preview_map> map) noexcept
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

    return save_preview_map(map, file_path);
}

preview_map_repository::error_type
preview_map_repository::save_preview_map(const std::shared_ptr<preview_map> &map,
                                         const std::string &file_path) const noexcept
{
    int fd = open(file_path.data(),
                  O_WRONLY | O_CREAT | O_TRUNC,
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (fd != -1) {
        std::vector<uchar> out_buff(map->size(), 0);
        cv::Mat in_mat(static_cast<int>(map->height_px()),
                       static_cast<int>(map->width_px()),
                       CV_8UC3,
                       map->data());

        const std::vector<int> params {
            cv::ImwriteFlags::IMWRITE_JPEG_QUALITY, 65
        };

        // TODO: exeption?
        bool res = cv::imencode(".jpg", in_mat, out_buff, params);

        if (res) {
            ssize_t written_size = write(fd, out_buff.data(),out_buff.size());
            if (written_size != -1) {
                // TODO: if a written size less than a preview size
                return error_type::none_error;
            } else {
                perror("write map to file");
                return error_type::file_creating_error;
            }
        } else {
            return error_type::file_creating_error;
        }
    } else {
        return error_type::file_creating_error;
    }
}

preview_map_repository::file_info
preview_map_repository::preview_file_info(const std::string& id,
                                          int64_t start_ut_msecs,
                                          const preview_map_format& format) const noexcept
{
    std::time_t start_ut_secs = start_ut_msecs / 1000;
    struct std::tm* utc_date = gmtime(&start_ut_secs);

    const int day = utc_date->tm_mday;
    const int month = utc_date->tm_mon + 1;
    const int year = utc_date->tm_year + 1900;

    const std::string dir1 = id;
    const std::string dir2 = std::to_string(year)
            + "-"
            + std::to_string(month)
            + "-" + std::to_string(day);
    const std::vector<std::string> relative_dir_path =  {dir1, dir2};

    const size_t map_number = (static_cast<size_t>(start_ut_msecs) % 86400000)
            / static_cast<size_t>(format.map_duration_msecs);

    std::string file_name = std::to_string(map_number)
            + "_"
            + std::to_string(format.item_duration_msecs)
            + "msecs";

    return {relative_dir_path, file_name};
}
