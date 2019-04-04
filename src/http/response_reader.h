#ifndef RESPONSE_READER_H
#define RESPONSE_READER_H

#include "response.h"

namespace http {

struct response_chunk
{
    const char* buff = nullptr;

    int file_d = -1;
    off_t file_offset = 0;

    size_t size = 0;
};

class response_reader
{
public:
    ~response_reader();

    bool init(const response& resp) noexcept;

    bool has_chunks() const noexcept;
    response_chunk get_chunk() const noexcept;
    void next(size_t size) noexcept;

private:
    inline static std::string status_code_to_str(int code) noexcept;

private:
    enum class state
    {
        read_none,
        read_line,
        read_body_str,
        read_body_buff,
        read_body_file
    };

private:
    response _resp;

    std::string _line;
    size_t _line_written_size = 0;

    int _body_fd = -1;
    size_t _body_size = 0;
    size_t _body_written_size = 0;

    state _state = state::read_line;
};

}

#endif // RESPONSE_READER_H
