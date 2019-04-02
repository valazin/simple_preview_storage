#ifndef PREVIEW_MAP_H
#define PREVIEW_MAP_H

#include <cstdint>
#include <cstddef>

class preview_map
{
public:
    preview_map(size_t number_of_rows,
                size_t number_of_columns,
                int64_t item_duration_msecs);

    bool add_preview(size_t row_number, size_t column_number);

private:
    const size_t _number_of_rows = 0;
    const size_t _number_of_columns = 0;
    const int64_t _item_duration_msecs = 0;
};

#endif // PREVIEW_MAP_H
