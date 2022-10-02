#pragma once

#include <algorithm>
#include <chrono>
#include <map>
#include <string>

namespace core {

template <typename T>
[[nodiscard]] inline T clamp(T min, T max, T val)
{
	return std::max<T>(min, std::min<T>(max, val));
}

[[nodiscard]] std::string gen_uuid();
[[nodiscard]] std::string to_string(const std::chrono::system_clock::time_point& tp);

} // namespace core