// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Hashed.hpp"
#include "Exception.hpp"
#include "../Crypto/Util.hpp"

#include <algorithm>
#include <istream>

namespace core {

sha256_t hashed_basic_streambuf::GetBlockHash() const
{
    sha256_t block_hash;
    CalcSHA256(block_hash, m_block);

    return block_hash;
}

int hashed_istreambuf::underflow()
{
    static constexpr std::array<uint8_t, 32> kEmptyHash = { { 0 } };

    if (gptr() == egptr())
    {
        BlockHeader header;
        m_src.read(reinterpret_cast<char*>(&header), sizeof(BlockHeader));

        if (header.block_index != m_block_index)
            throw descriptive_exception("Block index mismatch.");
        m_block_index++;

        m_block.clear();
        std::generate_n(std::back_inserter(m_block), header.block_size, [&]() { return static_cast<char>(m_src.get()); });

        if (!header.block_size)
        {
            if (header.block_hash != kEmptyHash)
                throw descriptive_exception("Corrupt EOS block.");

            return std::char_traits<char>::eof();
        }

        if (GetBlockHash() != header.block_hash)
            throw descriptive_exception("Block checksum error.");

        setg(m_block.data(), m_block.data(), m_block.data() + m_block.size());
    }

    return gptr() == egptr() ?
        std::char_traits<char>::eof() :
        std::char_traits<char>::to_int_type(*gptr());
}

bool hashed_ostreambuf::FlushBlock()
{
    static constexpr std::array<uint8_t, 32> kEmptyHash = { { 0 } };

    BlockHeader header;
    header.block_index = m_block_index++;
    header.block_hash = m_block.empty() ? kEmptyHash : GetBlockHash();
    header.block_size = static_cast<uint32_t>(m_block.size());

    m_dst.write(reinterpret_cast<const char*>(&header), sizeof(BlockHeader));
    m_dst.write(m_block.data(), m_block.size());
    if (!m_dst.good())
        return false;

    m_block.clear();
    return true;
}

int hashed_ostreambuf::overflow(int c)
{
    if (c == std::char_traits<char>::eof())
        return c;

    if (c > 0xff)
        throw descriptive_exception("char exeeded");

    m_block.push_back(static_cast<char>(c));

    if (m_block.size() == static_cast<size_t>(m_block_size) && !FlushBlock())
        return std::char_traits<char>::eof();

    return std::char_traits<char>::to_int_type(static_cast<char>(c));
}

int hashed_ostreambuf::sync()
{
    if (!m_block.empty() && !FlushBlock())
        return -1;

    return FlushBlock() ? 0 : -1;
}

} // namespace core