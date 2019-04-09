#ifndef PREVIEW_MAP_FORMAT_H
#define PREVIEW_MAP_FORMAT_H

#include <cstdint>
#include <cstddef>

struct preview_map_format
{
    size_t rows = 0;
    size_t cols = 0;
    int64_t item_duration_msecs = 0;
    size_t item_width_px = 0;
    size_t item_height_px = 0;
    size_t items = rows * cols;
    int64_t map_duration_msecs = static_cast<int64_t>(items) * item_duration_msecs;
};

#endif // PREVIEW_MAP_FORMAT_H
