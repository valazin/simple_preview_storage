#ifndef PREVIEW_MAP_H
#define PREVIEW_MAP_H

#include <cstdint>
#include <cstddef>

class preview_map
{
public:
    preview_map(size_t number_of_rows,
                size_t number_of_columns,
                size_t item_width,
                size_t item_height);

    bool add_preview(size_t item_number, char *buff, size_t size);

private:
    const size_t _number_of_rows = 0;
    const size_t _number_of_columns = 0;
    const size_t _item_width = 0;
    const size_t _item_height = 0;
    const size_t _item_size;

    bool _buff_was_allocated = false;
    char* _buff = nullptr;
};

#endif // PREVIEW_MAP_H
