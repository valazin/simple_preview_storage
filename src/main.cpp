#include <iostream>
#include <cstring>
#include <thread>

#include <opencv2/imgcodecs.hpp>

#include "api.h"
#include "preview_storage.h"

void calcuate_date()
{
    // TODO: move to utility
    constexpr int64_t total_secs = 1554091228 % 86400;
    constexpr int64_t secs = total_secs % 60;
    constexpr int64_t total_min = total_secs / 60;
    constexpr int64_t min = total_min % 60;
    constexpr int64_t hour = total_min / 60;

    std::cout << hour << " " << min << " " << secs << std::endl;
}

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

void insert_test1(preview_storage* storage)
{
    int64_t start_ut_msecs = 2*(60 * 60 * 24 * 1000);
    const int64_t duration_msecs = 10 * 1000;
    const size_t width = 160*2;
    const size_t height = 90*2;
    const size_t preview_size = 3*height*width;

    const int color_step = 255 / 30;

    // TODO: test without shared ptr
    for (int i=0; i<30000; ++i) {
        char* data = new char[preview_size];
        memset(data, color_step*(i+1), preview_size);

        storage->add_preview("id", start_ut_msecs, duration_msecs, width, height, data, preview_size);

        delete[] data;

        start_ut_msecs += duration_msecs;
    }

    std::cout << "job done" << std::endl;
}

int main()
{
    preview_storage* storage = new preview_storage("/tmp/preview-storage");

//    insert_test1(storage);

    api a(storage);
    if (!a.start("10.110.3.43", 1025)) {
        return -1;
    }

    while (true) {
        // TODO: flush none full maps if timeout
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return  0;
}
