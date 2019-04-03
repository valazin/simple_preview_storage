#ifndef PREVIEW_MAP_H
#define PREVIEW_MAP_H

#include <cstdint>
#include <cstddef>
#include <vector>

#include "preview_info.h"

class preview_map
{
public:
    preview_map(size_t number_of_rows,
                size_t number_of_columns,
                size_t item_width,
                size_t item_height);
    ~preview_map();

    bool is_full() const noexcept;

    bool insert_preview(size_t pos, char* buff, size_t size) noexcept;

private:
    const size_t c_number_of_rows = 0;
    const size_t c_number_of_columns = 0;
    const size_t c_number_of_previews = 0;
    const size_t c_item_width = 0;
    const size_t c_item_height = 0;
    const size_t c_item_size = 0;

    size_t _added_number_of_previews = 0;
    std::vector<preview_info> _preview_info;

    char* _buff = nullptr;
    bool _buff_was_allocated = false;
};

#endif // PREVIEW_MAP_H
