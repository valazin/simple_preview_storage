#ifndef PREVIEW_MAP_REPOSITORY_H
#define PREVIEW_MAP_REPOSITORY_H

#include <string>

#include "preview_map.h"

class preview_map_repository
{
public:
    enum error_type
    {
        none_error
    };

    preview_map_repository();

    error_type save(const std::string& id, std::shared_ptr<preview_map> map);

};

#endif // PREVIEW_MAP_REPOSITORY_H
