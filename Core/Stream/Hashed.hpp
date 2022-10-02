#pragma once

#include "../Crypto/Types.hpp"

#include <array>
#include <streambuf>
#include <vector>

namespace core {

class hashed_basic_streambuf
{
public:
    virtual ~hashed_basic_streambuf() = default;

protected:
    struct BlockHeader
    {
        uint32_t block_index{};
        sha256_t block_hash{};
        uint32_t block_size{};
    };

    uint32_t m_block_index{};
    std::vector<char> m_block;

    sha256_t GetBlockHash() const;
};

class hashed_istreambuf final
    : private hashed_basic_streambuf
    , public std::basic_streambuf<char, std::char_traits<char>>
{
public:
    hashed_istreambuf(std::istream& src)
        : m_src(src)
    {}

    virtual int underflow() override;

private:
    std::istream& m_src;
};

class hashed_ostreambuf final
    : private hashed_basic_streambuf
    , public std::basic_streambuf<char, std::char_traits<char>>
{
public:
    hashed_ostreambuf(std::ostream& dst)
        : m_dst(dst)
        , m_block_size(hashed_ostreambuf::DEFAULT_BLOCK_SIZE)
    {}
    hashed_ostreambuf(std::ostream& dst, const uint32_t block_size)
        : m_dst(dst)
        , m_block_size(block_size)
    {}

    virtual int overflow(int c) override;
    virtual int sync() override;

private:
    bool FlushBlock();

private:
    inline static constexpr uint32_t DEFAULT_BLOCK_SIZE = 1024 * 16;

private:
    std::ostream& m_dst;
    const uint32_t m_block_size;
};

} // namespace core