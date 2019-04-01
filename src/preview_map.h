#ifndef PREVIEW_MAP_H
#define PREVIEW_MAP_H

#include <cstdint>

class preview_map
{
public:
    preview_map(int row_number,
                int column_number,
                int64_t item_duration_msecs);

    bool add_preview(int row_index, int column_index);

private:
    const int _row_number = 0;
    const int _column_number = 0;
    const int64_t _item_duration_msecs = 0;
    const int64_t _total_duration_msecs = 0;
};

#endif // PREVIEW_MAP_H
