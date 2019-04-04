#ifndef PREVIEW_MAP_H
#define PREVIEW_MAP_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>

#include "http/buffer.h"

#include "preview_info.h"

class preview_map
{
public:
    preview_map(size_t number_of_rows,
                size_t number_of_columns,
                size_t preview_width,
                size_t preview_height) noexcept;
    ~preview_map();

    bool is_full() const noexcept;

    char* data() const noexcept;
    size_t size() const noexcept;

    size_t width_px() const;
    size_t height_px() const;

    bool insert_preview(size_t number, std::shared_ptr<http::buffer> buff) noexcept;

private:
    const size_t _rows = 0;
    const size_t _cols = 0;
    const size_t _number_of_previews = 0;
    const size_t _preview_width = 0;
    const size_t _preview_height = 0;
    const size_t _preview_size = 0;
    const size_t _map_size = 0;

    size_t _added_number_of_previews = 0;
    std::vector<preview_info> _preview_info;

    char* _buff = nullptr;
    bool _buff_was_allocated = false;
};

#endif // PREVIEW_MAP_H
