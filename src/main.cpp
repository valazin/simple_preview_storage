#include <iostream>

#include "preview_storage.h"

void print_map(int row, int column, int item_duration)
{
    int last_duration = 0;
    for (int i=0; i<row; ++i) {
        for (int j=0; j<column; ++j) {
            std::cout << last_duration << " - " << last_duration + item_duration << ";";
            last_duration += item_duration;
        }
        std::cout << std::endl;
    }
}

int main()
{
    // TODO: move to utility
    constexpr int64_t total_secs = 1554091228 % 86400;
    constexpr int64_t secs = total_secs % 60;
    constexpr int64_t total_min = total_secs / 60;
    constexpr int64_t min = total_min % 60;
    constexpr int64_t hour = total_min / 60;

    std::cout << hour << " " << min << " " << secs << std::endl;

    print_map(5,6,10);

    preview_storage storage;
    int64_t last_start_timestamp = 2*(60 * 60 * 24 * 1000);
    int64_t duration = 9 * 1000;
    size_t width = 160;
    size_t height = 80;
    for (int i=0; i<30; ++i) {
        storage.add_preview(last_start_timestamp, duration, width, height, nullptr, 0);
        last_start_timestamp += duration;
    }
    storage.add_preview(86395 * 1000, duration, width, height, nullptr, 0);

    return  0;
}
