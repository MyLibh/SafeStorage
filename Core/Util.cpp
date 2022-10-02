// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Util.hpp"

#include <random>

namespace core {

std::string gen_uuid()
{
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::uniform_int_distribution<int> dist(0, 15);

    const char* v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    std::string res;
    for (int i{}; i < 16; ++i)
    {
        if (dash[i])
            res += "-";

        res += v[dist(rng)];
        res += v[dist(rng)];
    }

    return res;
}

std::string to_string(const std::chrono::system_clock::time_point& tp)
{
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::string ts = std::ctime(&t);
    ts.resize(ts.size() - 1);
    return ts;
}

} // namespace core