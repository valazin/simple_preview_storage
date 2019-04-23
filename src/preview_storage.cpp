#include "preview_storage.h"

#include <iostream>

#include "preview_map_format.h"

preview_storage::preview_storage(const std::string &dir_path,
                                 int64_t map_flush_timeout_secs,
                                 int64_t map_release_timeout_secs) noexcept :
    _work_dir_path(dir_path),
    _map_flush_timeout_secs(map_flush_timeout_secs),
    _map_release_timeout_secs(map_release_timeout_secs)
{
    _repository = std::make_shared<preview_map_repository>(dir_path);
}

preview_storage::~preview_storage()
{
    _is_running = false;
    if (_garbage_thread.joinable()) {
        _garbage_thread.join();
    }
}

bool preview_storage::add_preview(const std::string& id,
                                  int64_t start_ut_msecs,
                                  int64_t duration_msecs,
                                  size_t width_px,
                                  size_t height_px,
                                  const char *data,
                                  size_t data_size) noexcept
{
    std::shared_ptr<private_builder> builder;

    auto search = _builders.find(id);
    if (search != _builders.end()) {
        builder = search->second;
    } else {
        const preview_map_format main_10sec{5,6,10000,320,180};
        const preview_map_format sub_1min{5,6,6*10000,160,90};
        const preview_map_format sub_1hour{4,6,360*10000,160,90};
        const std::vector<preview_map_format> sub_formats = {
            sub_1min,
            sub_1hour
        };

        builder = std::make_shared<private_builder>();
        builder->builder = std::make_unique<preview_map_builder>(main_10sec,
                                                                 sub_formats,
                                                                 _map_flush_timeout_secs * 1000);

        builder->builder->SaveMapHandler = [id, this](
                int64_t start_ut_msecs,
                const preview_map_format& format,
                std::shared_ptr<preview_map> map,
                const std::vector<preview_item_info>& items_info) {
            _repository->save(id, start_ut_msecs, format, map, items_info);
        };

        builder->builder->LoadMapHandler = [id, this] (
                int64_t start_ut_msecs,
                const preview_map_format& format) -> std::tuple<std::shared_ptr<preview_map>, std::vector<preview_item_info>> {
            auto [map, items_info, error] = _repository->load(id, start_ut_msecs, format);
            if (error != preview_map_repository::error_type::none_error) {
                return {map, items_info};
            }
            return {nullptr, {}};
        };

        _builders_mutex.lock();
        _builders.insert({id, builder});
        _builders_mutex.unlock();
    }

    builder->mutex.lock();
    builder->builder->add_preview(start_ut_msecs,
                                  duration_msecs,
                                  width_px,
                                  height_px,
                                  data,
                                  data_size);
    builder->mutex.unlock();

    // TODO: res
    return true;
}

std::map<std::string, std::string> preview_storage::get_metrics() const noexcept
{
    size_t active_maps_count = 0;
    auto i = _builders.begin();
    while (i != _builders.end()) {
        active_maps_count += i->second->builder->count();
        ++i;
    }

    std::map<std::string, std::string> res;
    res.insert({"active_maps_count", std::to_string(active_maps_count)});
    res.insert({"active_builders_count", std::to_string(_builders.size())});
    res.insert({"force_released_maps_count", std::to_string(_force_released_maps_count)});
    return res;
}

void preview_storage::start()
{
    _is_running = true;
    _garbage_thread = std::thread(&preview_storage::carbage_collector_loop, this);
}

void preview_storage::carbage_collector_loop() noexcept
{
    while (_is_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        size_t released_maps = 0;

        auto i = _builders.begin();
        while (i != _builders.end()) {
            i->second->mutex.lock();
            auto [count, error] = i->second->builder->release_maps(_map_release_timeout_secs * 1000);
            i->second->mutex.unlock();

            if (error == preview_map_builder::error_type::none_error) {
                released_maps += count;
            }

            if (i->second->builder->empty()) {
                _builders_mutex.lock();
                i = _builders.erase(i);
                _builders_mutex.unlock();
            } else {
                ++i;
            }
        }

        if (released_maps > 0) {
            _force_released_maps_count += released_maps;
            std::cout << released_maps << " maps were released by timeout" << std::endl;
        }
    }
}
