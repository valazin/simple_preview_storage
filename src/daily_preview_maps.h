#ifndef DAILY_PREVIEW_MAPS_H
#define DAILY_PREVIEW_MAPS_H

#include <cstdint>

class daily_preview_maps
{
public:
    daily_preview_maps(int64_t one_map_duration_msecs);

    bool add_preview(int64_t start_timestamp_msecs, int64_t duration_msecs);

private:
    int64_t _one_map_duration_msecs = 0;

};

#endif // DAILY_PREVIEW_MAPS_H
