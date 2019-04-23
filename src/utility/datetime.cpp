#include "datetime.h"

#include <ctime>

int64_t datetime::now_ut_msecs()
{
    return std::time(nullptr) * 1000;
}

date datetime::unix_timestamp_to_date(int64_t unix_timestamp)
{
    // TODO: release utc_data?
    struct std::tm* utc_date = gmtime(&unix_timestamp);

    date res;
    res.day = utc_date->tm_mday;
    res.month = utc_date->tm_mon + 1;
    res.year = utc_date->tm_year + 1900;

    return res;
}
