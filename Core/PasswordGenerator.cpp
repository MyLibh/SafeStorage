// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "PasswordGenerator.hpp"

#include <random>

namespace core {

inline constexpr std::string_view CAPITALS    = "ABCDEFGHIGKLMNOPQRSTUVWXYZ";
inline constexpr std::string_view LOWERCASE   = "abcdefghijklmnopqrstuvwxyz";
inline constexpr std::string_view DIGITS      = "0123456789";
inline constexpr std::string_view SPECIAL1    = "#$%&@^`~";
inline constexpr std::string_view SPECIAL2    = ".,:;";
inline constexpr std::string_view SPECIAL3    = "\"'";
inline constexpr std::string_view SPECIAL4    = "/|_-";
inline constexpr std::string_view SPECIAL5    = "<*+!?=";
inline constexpr std::string_view PARENTHESES = "{[()]}";

std::string PasswordGenerator::GeneratePassword() const
{
    thread_local static std::mt19937 rg{ std::random_device{}() };

    std::string dict;
    dict.reserve(sizeof(uint8_t) >> 1);
    if (m_flags.test(static_cast<size_t>(Flag::Capitals)))    dict += CAPITALS;
    if (m_flags.test(static_cast<size_t>(Flag::Lowercase)))   dict += LOWERCASE;
    if (m_flags.test(static_cast<size_t>(Flag::Digits)))      dict += DIGITS;
    if (m_flags.test(static_cast<size_t>(Flag::Special1)))    dict += SPECIAL1;
    if (m_flags.test(static_cast<size_t>(Flag::Special2)))    dict += SPECIAL2;
    if (m_flags.test(static_cast<size_t>(Flag::Special3)))    dict += SPECIAL3;
    if (m_flags.test(static_cast<size_t>(Flag::Special4)))    dict += SPECIAL4;
    if (m_flags.test(static_cast<size_t>(Flag::Special5)))    dict += SPECIAL5;
    if (m_flags.test(static_cast<size_t>(Flag::Parentheses))) dict += PARENTHESES;

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
    size_t res{};
    if (m_flags.test(static_cast<size_t>(Flag::Capitals)))    res += CAPITALS.length();
    if (m_flags.test(static_cast<size_t>(Flag::Lowercase)))   res += LOWERCASE.length();
    if (m_flags.test(static_cast<size_t>(Flag::Digits)))      res += DIGITS.length();
    if (m_flags.test(static_cast<size_t>(Flag::Special1)))    res += SPECIAL1.length();
    if (m_flags.test(static_cast<size_t>(Flag::Special2)))    res += SPECIAL2.length();
    if (m_flags.test(static_cast<size_t>(Flag::Special3)))    res += SPECIAL3.length();
    if (m_flags.test(static_cast<size_t>(Flag::Special4)))    res += SPECIAL4.length();
    if (m_flags.test(static_cast<size_t>(Flag::Special5)))    res += SPECIAL5.length();
    if (m_flags.test(static_cast<size_t>(Flag::Parentheses))) res += PARENTHESES.length();

    return static_cast<uint8_t>(res);
}

} // namespace core