#include "preview_storage.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <ctime>
#include <cassert>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "filesystem.h"

preview_storage::preview_storage(const std::string &dir_path) noexcept :
    _work_dir_path(dir_path)
{
}

bool preview_storage::add_preview(const std::string &id,
                                  int64_t start_ut_msecs,
                                  int64_t duration_msecs,
                                  size_t width,
                                  size_t height,
                                  const char *data,
                                  size_t data_size) noexcept
{
    std::shared_ptr<preview_map_builder> builder;

    auto search = _builders.find(id);
    if (search != _builders.end()) {
        builder = search->second;
    } else {
        const preview_map_builder::map_format main_10sec{5,6,10000,320,180};
        const preview_map_builder::map_format sub_1min{5,6,6*10000,160,90,};
        const preview_map_builder::map_format sub_1hour{4,6,360*10000,160,90};
        const std::vector<preview_map_builder::map_format> sub_formats = {
            sub_1min,
            sub_1hour
        };
        preview_map_builder::task task {main_10sec, sub_formats};

        builder = std::make_shared<preview_map_builder>(task);
        builder->MapBuildedHandler = [this](
                int64_t start_ut_msecs,
                const preview_map_builder::map_format& format,
                std::shared_ptr<preview_map> map) {

        };

        _builders.insert({id, builder});
    }

    builder->insert(start_ut_msecs,
                    duration_msecs,
                    width, height,
                    data,
                    data_size);

    // TODO: res
    return true;
}

bool preview_storage::save_preview_map(const std::shared_ptr<preview_map>& map,
                                       const std::string& file_path) const noexcept
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
                return true;
            } else {
                perror("write map to file");
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
}

bool preview_storage::create_preview_dir(const std::string& id,
                                         size_t day_number) const noexcept
{
    std::string dir_path = preview_dir_path(id, day_number);
    if (filesystem::dir_is_exist(dir_path)) {
        return true;
    } else {
        dir_path = _work_dir_path + "/" + id;
        if (!filesystem::dir_is_exist(dir_path)
                && !filesystem::create_directory(dir_path)) {
            return false;
        }
        dir_path += "/" + std::to_string(day_number);
        if (!filesystem::dir_is_exist(dir_path)
                && !filesystem::create_directory(dir_path)) {
            return false;
        }
        return true;
    }
}

std::string preview_storage::preview_dir_path(const std::string &id,
                                              size_t day_number) const noexcept
{
    return _work_dir_path + "/" + id + "/"+ std::to_string(day_number);
}

std::string preview_storage::preview_file_name(size_t map_number) const noexcept
{
    return std::to_string(map_number);
}

std::string preview_storage::preview_file_path(const std::string &id,
                                               size_t day_number,
                                               size_t map_number) const noexcept
{
    return preview_dir_path(id, day_number) + "/" + preview_file_name(map_number);
}
