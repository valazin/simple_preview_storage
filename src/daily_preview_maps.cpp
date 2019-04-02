#include "daily_preview_maps.h"

daily_preview_maps::daily_preview_maps(int64_t one_map_duration_msecs) :
    _one_map_duration_msecs(one_map_duration_msecs)
{
}

bool daily_preview_maps::add_preview(int64_t start_timestamp_msecs,
                                     int64_t duration_msecs)
{
    return false;
}
