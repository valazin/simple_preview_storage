#include "preview_map_repository.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctime>

#include <fstream>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "filesystem.h"
#include "string_utils.h"

preview_map_repository::preview_map_repository(const std::string& dir_path) noexcept :
      _dir_path(dir_path)
{
    if (!filesystem::dir_is_exist(dir_path)
            && !filesystem::create_path(dir_path)) {
        std::cerr << "Error create dir_path for preview_map_repository";
    }
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

std::tuple<std::shared_ptr<preview_map>, std::vector<preview_item_info>, preview_map_repository::error_type>
preview_map_repository::load(const std::string &id, int64_t start_ut_msecs, const preview_map_format &format) noexcept
{
    const file_info info = preview_file_info(id, start_ut_msecs, format);
    std::string dir_path = _dir_path;
    for (auto& dir : info.relative_dir_path) {
        dir_path += "/" + dir;
    }
    if (!filesystem::dir_is_exist(dir_path)) {
        std::cerr << "couldn't load from " << dir_path;
        return {nullptr, std::vector<preview_item_info>(), error_type::file_load_error};
    }

    const std::string file_path = dir_path + "/" + info.file_name;
    const std::string meta_file_path = file_path + "_meta";

    auto [items_info, error] = load_preview_offsets_from_file(meta_file_path);
    if (error != error_type::none_error) {
        return {nullptr, std::vector<preview_item_info>(), error};
    }
    if (items_info.size()!=format.items) {
        return {nullptr, std::vector<preview_item_info>(), error_type::error_parse_meta_info};
    }

    auto [map, error_map] = load_preview_map_from_file(file_path, format, items_info);
    return {map, items_info, error_map};
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

std::tuple<std::shared_ptr<preview_map>, preview_map_repository::error_type>
preview_map_repository::load_preview_map_from_file(const std::string &file_path,
                                                   const preview_map_format &format,
                                                   const std::vector<preview_item_info> &items_info) noexcept
{
    size_t items_count(0);
    for(auto i : items_info)
    {
        if (!i.empty)
            ++items_count;
    }
    cv::Mat image = cv::imread(file_path.c_str(), cv::IMREAD_COLOR);
    auto result = std::make_shared<preview_map>(format.rows,
                                                format.cols,
                                                format.item_width_px,
                                                format.item_height_px,
                                                reinterpret_cast<char*>(image.data),
                                                items_count);

    return {result, error_type::none_error};
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

std::tuple<std::vector<preview_item_info>, preview_map_repository::error_type>
preview_map_repository::load_preview_offsets_from_file(const std::string& file_path) noexcept
{
    std::vector<preview_item_info> result;
    auto [data, data_size, error] = load_from_file(file_path);
    if (error!=error_type::none_error) {
        return {result, error};
    }

    std::string text(data, data_size);
    for (auto s : string_utils::split_string(text, ';')) {
        if (string_utils::string_is_number(s)) {
            result.push_back(preview_item_info{false, std::stol(s)*1000});
        } else if (s=="null") {
            result.push_back(preview_item_info{true, 0});
        } else {
            std::cerr << "item is no valid. bad data";
            result.push_back(preview_item_info{true, 0});
        }
        
    }
    return {result, error_type::none_error};
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

    const size_t map_number_of_day = (static_cast<size_t>(start_ut_msecs) % 86400000)
                                     / static_cast<size_t>(format.map_duration_msecs);

    std::string file_name = std::to_string(map_number_of_day)
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


std::tuple<char*, size_t, preview_map_repository::error_type>
preview_map_repository::load_from_file(const std::string& file_path) noexcept
{
    std::ifstream file(file_path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "open file to read: " << file_path;
        return {nullptr, 0, error_type::file_load_error};
    }

    std::streampos data_size = file.tellg();
    file.seekg (0, std::ios::beg);

    auto data = new char[static_cast<size_t>(data_size)];
    file.read(data, data_size);

    file.close();
    return {data, static_cast<size_t>(data_size), error_type::none_error};
}
