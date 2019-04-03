#include <iostream>
#include <cstring>

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

    preview_storage storage("/tmp/preview-storage");
    int64_t start_ut_msecs = 2*(60 * 60 * 24 * 1000);
    int64_t duration_msecs = 9 * 1000;
    size_t width = 160;
    size_t height = 80;

    const size_t data_size = width*height*3;

    char* black_data = new char[data_size];
    memset(black_data, 0, data_size);

    char* white_data = new char[data_size];
    memset(white_data, 255, data_size);

    for (int i=0; i<600; ++i) {
        char* data = nullptr;
        if (i%2) {
            data = black_data;
        } else {
            data = white_data;
        }
        storage.add_preview(start_ut_msecs, duration_msecs, width, height, data, data_size);
        start_ut_msecs += duration_msecs;
    }
    storage.add_preview(86395 * 1000, duration_msecs, width, height, white_data, data_size);

    return  0;
}
