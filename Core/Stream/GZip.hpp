#pragma once

#include <array>
#include <istream>
#include <ostream>
#include <vector>

#include <zlib.h>

namespace core {

class gzip_istreambuf final : public std::basic_streambuf<char, std::char_traits<char>>
{
public:
    gzip_istreambuf(std::istream& src);
    ~gzip_istreambuf();

    virtual int underflow() override;

private:
    inline static constexpr size_t BUFFER_SIZE{ 16384 };

private:
    std::istream& m_src;
    z_stream m_z_stream;

    std::array<char, gzip_istreambuf::BUFFER_SIZE> m_input;
    std::array<char, gzip_istreambuf::BUFFER_SIZE> m_output;
};

class gzip_ostreambuf final : public std::basic_streambuf<char, std::char_traits<char>>
{
public:
    gzip_ostreambuf(std::ostream& dst);
    ~gzip_ostreambuf();

    virtual int overflow(int c) override;
    virtual int sync() override;

private:
    bool WriteOutput(bool flush);

private:
    inline static constexpr size_t BUFFER_SIZE{ 16384 };

private:
    std::ostream& m_dst;
    z_stream m_z_stream;

    std::vector<char> m_buffer;
};

} // namespace core