#ifndef PREVIEW_MAP_H
#define PREVIEW_MAP_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <memory>

class preview_map
{
public:
    preview_map(size_t rows,
                size_t cols,
                size_t item_width_px,
                size_t item_height_px) noexcept;
    preview_map(size_t rows,
                size_t cols,
                size_t item_width_px,
                size_t item_height_px,
                const char *buff,
                size_t items_count) noexcept;
    ~preview_map();

    bool is_full() const noexcept;

    const char* data() const noexcept;
    size_t size() const noexcept;

    size_t width_px() const;
    size_t height_px() const;

    bool insert(size_t number,
                const char* buff,
                std::size_t size) noexcept;

private:
    const size_t _rows = 0;
    const size_t _cols = 0;
    const size_t _items = 0;
    const size_t _item_width_px = 0;
    const size_t _item_height_px = 0;
    const size_t _item_size = 0;
    const size_t _map_size = 0;

    size_t _added_number_of_previews = 0;

    char* _buff = nullptr;
    bool _buff_was_allocated = false;
};

#endif // PREVIEW_MAP_H
