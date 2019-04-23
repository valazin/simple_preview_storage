#ifndef PREVIEW_STORAGE_H
#define PREVIEW_STORAGE_H

#include <cstdint>
#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <unordered_map>

#include "preview_map.h"
#include "preview_map_builder.h"
#include "preview_map_repository.h"

class preview_storage
{
public:
    preview_storage(const std::string& dir_path,
                    int64_t map_flush_timeout_secs,
                    int64_t map_release_timeout_secs) noexcept;
    ~preview_storage();

    bool add_preview(const std::string& id,
                     int64_t start_ut_msecs,
                     int64_t duration_msecs,
                     size_t width_px,
                     size_t height_px,
                     const char* data,
                     size_t data_size) noexcept;

    std::map<std::string, std::string> get_metrics() const noexcept;

    void start();

private:
    struct private_builder
    {
        std::mutex mutex;
        std::unique_ptr<preview_map_builder> builder;
    };

private:
    void carbage_collector_loop() noexcept;

private:
    const std::string _work_dir_path;
    const int64_t _map_flush_timeout_secs = 0;
    const int64_t _map_release_timeout_secs = 0;

    std::shared_ptr<preview_map_repository> _repository;

    std::mutex _builders_mutex;
    std::unordered_map<std::string, std::shared_ptr<private_builder>> _builders;

    std::atomic<bool> _is_running = false;
    std::thread _garbage_thread;

    std::atomic<size_t> _force_released_maps_count = 0;
};

#endif // PREVIEW_STORAGE_H
