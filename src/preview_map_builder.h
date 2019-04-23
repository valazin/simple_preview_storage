#ifndef PREVIEW_MAP_BUILDER_H
#define PREVIEW_MAP_BUILDER_H

#include <functional>
#include <unordered_map>

#include "preview_map.h"
#include "preview_map_format.h"
#include "preview_item_info.h"
#include "preview_map_repository.h"

class preview_map_builder
{
public:
    enum error_type
    {
        none_error,
        invalid_arguments
    };

    std::function<
    void (int64_t start_ut_msecs,
          const preview_map_format& format,
          std::shared_ptr<preview_map> map,
          const std::vector<preview_item_info>& items_info)
    > SaveMapHandler;

    std::function<
    std::tuple<std::shared_ptr<preview_map>, std::vector<preview_item_info>>
    (int64_t start_ut_msecs, const preview_map_format& format )
    > LoadMapHandler;

    preview_map_builder(const preview_map_format& main_format,
                        const std::vector<preview_map_format>& sub_formats,
                        int64_t flush_timeout_msecs) noexcept;

    error_type add_preview(int64_t start_ut_msecs,
                           int64_t duration_msecs,
                           size_t width_px,
                           size_t height_px,
                           const char* data,
                           size_t data_size) noexcept;

    bool empty() const noexcept;

    std::tuple<size_t, error_type>
    release_maps(int64_t unmodified_secs) noexcept;

private:
    struct private_map
    {
        size_t item_counter = 0;
        size_t flush_counter = 0;
        int64_t last_modyfied_ut = 0;
        int64_t start_ut_msecs = 0;
        bool has_unsave_changes = false;
        std::shared_ptr<preview_map> map;
        std::vector<preview_item_info> items_info;
    };

    struct private_format
    {
        preview_map_format format;
        std::unordered_map<size_t, std::shared_ptr<private_map>> maps;
    };

private:
    void insert(int64_t start_ut_msecs,
                const char* data,
                size_t data_size,
                private_format& format) noexcept;

    inline bool map_is_builded(std::shared_ptr<private_map> map,
                               const private_format& format) const noexcept;
    inline bool map_is_ready_to_flush_by_timeout(std::shared_ptr<private_map> map,
                                      const private_format& format) const noexcept;

private:
    const int64_t  _flush_timeout_msecs = 0;
    private_format _main_format;
    std::vector<private_format> _sub_formats;
};

#endif // PREVIEW_MAP_BUILDER_H
