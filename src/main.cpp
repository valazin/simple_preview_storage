#include <iostream>
#include <cstring>
#include <thread>
#include <memory.h>

#include <glog/logging.h>

#include "api.h"
#include "preview_storage.h"
#include "env_settings.h"

void insert_test1(std::shared_ptr<preview_storage> storage)
{
    int64_t start_ut_msecs = 2*(60 * 60 * 24 * 1000);
    const int64_t duration_msecs = 10 * 1000;
    const size_t width = 160*2;
    const size_t height = 90*2;
    const size_t preview_size = 3*height*width;

    const int color_step = 255 / 30;

    // TODO: test without shared ptr
    for (int i=0; i<29; ++i) {
//        if (i % 6 == 0) {
//            start_ut_msecs += duration_msecs;
//            continue;
//        }

        char* data = new char[preview_size];
        memset(data, color_step*(i+1), preview_size);

        storage->add_preview("id", start_ut_msecs, duration_msecs, width, height, data, preview_size);

        delete[] data;

        start_ut_msecs += duration_msecs;
    }

    std::cout << "job done" << std::endl;
}

int main(int argc, char* argv[])
{
    //configure logging
    google::InitGoogleLogging(argv[0]);
    FLAGS_logtostderr = 1; //log all messages to console

    auto settings = std::make_unique<env_settings>();

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

    auto storage = std::make_shared<preview_storage>(storage_conf.path,
                                                     storage_conf.map_flush_duration_secs,
                                                     storage_conf.map_release_timeout_secs);
    storage->start();

    api a(storage);
    if (!a.start(http_conf.host, http_conf.port)) {
        return -1;
    }

    insert_test1(storage);

    while (true) {
        // TODO: flush none full maps if timeout
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return  0;
}
