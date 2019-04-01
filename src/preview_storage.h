#ifndef PREVIEW_STORAGE_H
#define PREVIEW_STORAGE_H

#include <cstdint>
#include <array>

#include "daily_preview_maps.h"

class preview_storage
{
public:
    preview_storage();

    bool add_preview(int64_t start_timestamp_msecs, int64_t duration);

private:
    std::size_t get_day_number(int64_t timestamp_msecs) const;
    int64_t get_day_and_normalize(int64_t timestamp_msecs) const;

private:
    const size_t current_day = 0;

    static const std::size_t max_days = 3;
    std::array<daily_preview_maps, max_days> _daily_preview_maps_array;

};

#endif // PREVIEW_STORAGE_H
