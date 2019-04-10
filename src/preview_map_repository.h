#ifndef PREVIEW_MAP_REPOSITORY_H
#define PREVIEW_MAP_REPOSITORY_H

#include <string>

#include "preview_map.h"
#include "preview_map_format.h"
#include "preview_item_info.h"

class preview_map_repository
{
public:
    enum error_type
    {
        none_error,
        file_creating_error,
        image_encoding_error
    };

    explicit preview_map_repository(const std::string& dir_path) noexcept;

    error_type save(const std::string& id,
                    int64_t start_ut_msecs,
                    const preview_map_format& format,
                    std::shared_ptr<preview_map> map,
                    const std::vector<preview_item_info>& items_info) noexcept;

private:
    struct file_info
    {
        const std::vector<std::string> relative_dir_path;
        const std::string file_name;
    };

private:
    static error_type save_preview_map(const std::shared_ptr<preview_map>& map,
                                       const std::string& file_path) noexcept;

    static error_type save_preview_offsets(const std::vector<preview_item_info>& items_info,
                                           const std::string& file_path) noexcept;

    static file_info preview_file_info(const std::string& id,
                                       int64_t start_ut_msecs,
                                       const preview_map_format& format) noexcept;

    static error_type save_to_file(const void* data,
                                   size_t data_size,
                                   const std::string &file_path) noexcept;

private:
    const std::string _dir_path;
};

#endif // PREVIEW_MAP_REPOSITORY_H
