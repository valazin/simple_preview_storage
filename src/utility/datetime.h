#ifndef DATETIME_H
#define DATETIME_H

#include <cstdint>

struct date
{
    int day = 0;
    int month = 0;
    int year = 0;
};

class datetime
{
public:
    static int64_t unix_timestamp();
    static date unix_timestamp_to_date(int64_t unix_timestamp);
};

#endif // DATETIME_H
