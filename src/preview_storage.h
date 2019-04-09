#ifndef PREVIEW_STORAGE_H
#define PREVIEW_STORAGE_H

#include <cstdint>
#include <vector>
#include <memory>
#include <unordered_map>

#include "preview_map.h"
#include "preview_map_builder.h"

class preview_storage
{
public:
    explicit preview_storage(const std::string& dir_path) noexcept;

    bool add_preview(const std::string& id,
                     int64_t start_ut_msecs,
                     int64_t duration_msecs,
                     size_t width,
                     size_t height,
                     const char* data,
                     size_t data_size) noexcept;

private:
    bool save_preview_map(const std::shared_ptr<preview_map>& map,
                          const std::string& file_path) const noexcept;

    bool create_preview_dir(const std::string& id,
                            size_t day_number) const noexcept;

    std::string preview_dir_path(const std::string& id,
                                 size_t day_number) const noexcept;
    std::string preview_file_name(size_t map_number) const noexcept;
    std::string preview_file_path(const std::string& id,
                                  size_t day_number,
                                  size_t map_number) const noexcept;

private:
    const std::string _work_dir_path;
    std::unordered_map<std::string, std::shared_ptr<preview_map_builder>> _builders;
};

#endif // PREVIEW_STORAGE_H
