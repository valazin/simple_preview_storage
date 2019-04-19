#include "filesystem.h"
#include "string_utils.h"

#include <sys/stat.h>
#include <fcntl.h>

ssize_t filesystem::file_size(int fd)
{
    struct stat st;
    if (fstat(fd, &st) != -1) {
        return static_cast<ssize_t>(st.st_size);
    }
    return -1;
}

bool filesystem::dir_is_exist(const std::string& path)
{
    struct stat st;
    if (stat(path.data(), &st) != -1) {
        return (st.st_mode & S_IFMT) == S_IFDIR;
    } else {
        if (errno == ENOENT) {
            return false;
        }
        // fatal errol
        perror("stat while check dir");
    }
    return false;
}

bool filesystem::create_directory(const std::string& path)
{
    if (mkdir(path.data(), 0700) != -1) {
        return true;
    } else {
        if (errno == EEXIST) {
            return true;
        }
        perror("mkdir");
    }
    return false;
}

bool filesystem::create_path(const std::string& absolute_path)
{
    if (!dir_is_exist(absolute_path)) {
        auto dirs = string_utils::split_string(absolute_path, '/');
        std::string s;
        for (auto&& dir : dirs) {
            if (dir.empty()) {
                continue;
            }
            s += "/" + dir;
            if (!filesystem::dir_is_exist(s)
                && !filesystem::create_directory(s)) {
                return false;
            }
        }
    }
    return true;
}
