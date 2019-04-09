#include "preview_storage.h"

#include <ctime>
#include <cassert>
#include <iostream>

#include "filesystem.h"
#include "preview_map_format.h"

preview_storage::preview_storage(const std::string &dir_path) noexcept :
    _work_dir_path(dir_path)
{
    _repository = std::make_shared<preview_map_repository>(dir_path);
}

bool preview_storage::add_preview(const std::string& id,
                                  int64_t start_ut_msecs,
                                  int64_t duration_msecs,
                                  size_t width_px,
                                  size_t height_px,
                                  const char *data,
                                  size_t data_size) noexcept
{
    std::shared_ptr<preview_map_builder> builder;

    auto search = _builders.find(id);
    if (search != _builders.end()) {
        builder = search->second;
    } else {
        const preview_map_format main_10sec{5,6,10000,320,180};
        const preview_map_format sub_1min{5,6,6*10000,160,90,};
        const preview_map_format sub_1hour{4,6,360*10000,160,90};
        const std::vector<preview_map_format> sub_formats = {
            sub_1min,
            sub_1hour
        };

        builder = std::make_shared<preview_map_builder>(main_10sec, sub_formats);

        builder->MapBuildedHandler = [id, this](
                int64_t start_ut_msecs,
                const preview_map_format& format,
                std::shared_ptr<preview_map> map) {
            _repository->save(id, start_ut_msecs, format, map);
        };

        _builders.insert({id, builder});
    }

    builder->add_preview(start_ut_msecs,
                         duration_msecs,
                         width_px,
                         height_px,
                         data,
                         data_size);

    // TODO: res
    return true;
}
