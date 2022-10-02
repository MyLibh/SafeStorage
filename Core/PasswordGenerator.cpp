// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "PasswordGenerator.hpp"

#include <random>

namespace core {

std::string PasswordGenerator::GeneratePassword() const
{
    thread_local static std::mt19937 rg{ std::random_device{}() };

    std::string dict;
    dict.reserve(sizeof(uint8_t) >> 1);
    if (m_flags.test(static_cast<size_t>(Flag::Capitals)))    dict += "ABCDEFGHIGKLMNOPQRSTUVWXYZ";
    if (m_flags.test(static_cast<size_t>(Flag::Lowercase)))   dict += "abcdefghijklmnopqrstuvwxyz";
    if (m_flags.test(static_cast<size_t>(Flag::Digits)))      dict += "0123456789";
    if (m_flags.test(static_cast<size_t>(Flag::Special1)))    dict += "#$%&@^`~";
    if (m_flags.test(static_cast<size_t>(Flag::Special2)))    dict += ".,:;";
    if (m_flags.test(static_cast<size_t>(Flag::Special3)))    dict += "\"'";
    if (m_flags.test(static_cast<size_t>(Flag::Special4)))    dict += "/|_-";
    if (m_flags.test(static_cast<size_t>(Flag::Special5)))    dict += "<*+!?=";
    if (m_flags.test(static_cast<size_t>(Flag::Parentheses))) dict += "{[()]}";

    if (dict.empty())
        return {};

    std::uniform_int_distribution<std::string::size_type> pick(0, dict.length() - 1);

    std::string s;
    s.reserve(m_length);
    for (uint8_t i{}; i < m_length; ++i)
        s += dict[pick(rg)];

    return s;
}

uint8_t PasswordGenerator::GetPoolSize() const
{
    uint8_t res{};
    if (m_flags.test(static_cast<size_t>(Flag::Capitals)))    res += 26;
    if (m_flags.test(static_cast<size_t>(Flag::Lowercase)))   res += 26;
    if (m_flags.test(static_cast<size_t>(Flag::Digits)))      res += 10;
    if (m_flags.test(static_cast<size_t>(Flag::Special1)))    res += 8;
    if (m_flags.test(static_cast<size_t>(Flag::Special2)))    res += 4; // -V112
    if (m_flags.test(static_cast<size_t>(Flag::Special3)))    res += 2;
    if (m_flags.test(static_cast<size_t>(Flag::Special4)))    res += 4; // -V112
    if (m_flags.test(static_cast<size_t>(Flag::Special5)))    res += 6;
    if (m_flags.test(static_cast<size_t>(Flag::Parentheses))) res += 6;

    return res;
}

} // namespace core