#include <iostream>
#include <cstring>
#include <thread>

#include <opencv2/imgcodecs.hpp>

#include "api.h"
#include "preview_storage.h"
#include "env_settings.h"

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
    auto settings = new env_settings;

    auto [http_conf, http_ok] = settings->get_http_server_settings();
    if (!http_ok) {
        std::cerr << "couldn't load http settings" << std::endl;
        return -1;
    }

    auto [storage_conf, storage_ok] = settings->get_preview_storage_settings();
    if (!storage_ok) {
        std::cerr << "couldn't load storage settings" << std::endl;
        return -1;
    }

    preview_storage* storage = new preview_storage(storage_conf.path);

    api a(storage);
    if (!a.start(http_conf.host, http_conf.port)) {
        return -1;
    }

    while (true) {
        // TODO: flush none full maps if timeout
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return  0;
}
