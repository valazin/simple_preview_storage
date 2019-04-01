#include <iostream>



int main()
{
    constexpr int64_t total_secs = 1554091228 % 86400;
    constexpr int64_t secs = total_secs % 60;
    constexpr int64_t total_min = total_secs / 60;
    constexpr int64_t min = total_min % 60;
    constexpr int64_t hour = total_min / 60;

    std::cout << hour << " " << min << " " << secs << std::endl;

    return  0;
}
