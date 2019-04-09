#ifndef PREVIEW_MAP_REPOSITORY_H
#define PREVIEW_MAP_REPOSITORY_H

#include <string>

#include "preview_map.h"
#include "preview_map_format.h"

class preview_map_repository
{
public:
    enum error_type
    {
        none_error,
        file_creating_error
    };

    explicit preview_map_repository(const std::string& dir_path) noexcept;

    error_type save(const std::string& id,
                    int64_t start_ut_msecs,
                    const preview_map_format& format,
                    std::shared_ptr<preview_map> map) noexcept;

private:
    struct file_info
    {
        const std::vector<std::string> relative_dir_path;
        const std::string file_name;
    };

private:
    error_type save_preview_map(const std::shared_ptr<preview_map>& map,
                                const std::string& file_path) const noexcept;

    file_info preview_file_info(const std::string& id,
                                int64_t start_ut_msecs,
                                const preview_map_format& format) const noexcept;

private:
    const std::string _dir_path;

};

#endif // PREVIEW_MAP_REPOSITORY_H
