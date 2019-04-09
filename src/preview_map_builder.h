#ifndef PREVIEW_MAP_BUILDER_H
#define PREVIEW_MAP_BUILDER_H

#include <functional>
#include <unordered_map>

#include "preview_map.h"

class preview_map_builder
{
public:
    enum error_type
    {
        none_error,
        invalid_arguments
    };

    struct map_format
    {
        const size_t rows = 0;
        const size_t cols = 0;
        const int64_t item_duration_msecs = 0;
        const size_t item_width_px = 0;
        const size_t item_height_px = 0;
        const size_t items = rows * cols;
        const int64_t map_duration_msecs = static_cast<int64_t>(items) * item_duration_msecs;

        // TODO: move to private structure
        std::unordered_map<size_t, std::shared_ptr<preview_map>> maps;
    };

    struct task
    {
        map_format format;
        std::vector<map_format> sub_formats;
    };

    std::function<void (int64_t start_ut_msecs,
                        const map_format& format,
                        std::shared_ptr<preview_map> map)> MapBuildedHandler;

    explicit preview_map_builder(task tsk) noexcept;

    error_type insert(int64_t start_ut_msecs,
                      int64_t duration_msecs,
                      size_t width_px,
                      size_t height_px,
                      const char* data,
                      size_t data_size) noexcept;

private:
    void private_insert(int64_t start_ut_msecs,
                        const char *data,
                        size_t data_size,
                        map_format& format) noexcept;

private:
    task _task;
};

#endif // PREVIEW_MAP_BUILDER_H
