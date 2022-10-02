// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "GZip.hpp"
#include "Exception.hpp"

#include <string>

namespace core {

gzip_istreambuf::gzip_istreambuf(std::istream& src)
    : m_src(src)
{
    m_z_stream.zalloc = Z_NULL;
    m_z_stream.zfree = Z_NULL;
    m_z_stream.opaque = Z_NULL;
    m_z_stream.avail_in = 0;
    m_z_stream.next_in = reinterpret_cast<uint8_t*>(m_input.data());
    m_z_stream.avail_out = static_cast<uInt>(m_output.size());
    m_z_stream.next_out = reinterpret_cast<uint8_t*>(m_output.data());

    if (inflateInit2(&m_z_stream, 16 + MAX_WBITS) != Z_OK)
        return;
}

gzip_istreambuf::~gzip_istreambuf()
{
    inflateEnd(&m_z_stream);
}

int gzip_istreambuf::underflow() {
    if (gptr() == egptr())
    {
        if (m_z_stream.avail_in == 0)
        {
            if (!m_src.good())
                return std::char_traits<char>::eof();

            m_src.read(m_input.data(), m_input.size());

            m_z_stream.avail_in = static_cast<uInt>(m_src.gcount());
            m_z_stream.next_in = reinterpret_cast<uint8_t*>(m_input.data());

            if (m_z_stream.avail_in < 1)
                return std::char_traits<char>::eof();
        }

        m_z_stream.avail_out = static_cast<uInt>(m_output.size());
        m_z_stream.next_out = reinterpret_cast<uint8_t*>(m_output.data());

        int res = inflate(&m_z_stream, Z_NO_FLUSH);
        if (res < 0)
            throw descriptive_exception("Gzip inflation error");


        std::size_t output_bytes = m_output.size() - static_cast<size_t>(m_z_stream.avail_out);
        setg(m_output.data(), m_output.data(), m_output.data() + output_bytes);
    }

    return gptr() == egptr() ?
        std::char_traits<char>::eof() :
        std::char_traits<char>::to_int_type(*gptr());
}

gzip_ostreambuf::gzip_ostreambuf(std::ostream& dst)
    : m_dst(dst)
{
    m_z_stream.zalloc = Z_NULL;
    m_z_stream.zfree = Z_NULL;
    m_z_stream.opaque = Z_NULL;
    m_z_stream.avail_in = 0;
    m_z_stream.next_in = Z_NULL;
    m_z_stream.avail_out = 0;
    m_z_stream.next_out = Z_NULL;

    if (deflateInit2(&m_z_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK)
        return;
}

gzip_ostreambuf::~gzip_ostreambuf()
{
    deflateEnd(&m_z_stream);
}

bool gzip_ostreambuf::WriteOutput(bool flush)
{
    std::vector<char> out(gzip_ostreambuf::BUFFER_SIZE);

    m_z_stream.avail_in = static_cast<uInt>(m_buffer.size());
    m_z_stream.next_in = reinterpret_cast<uint8_t*>(m_buffer.data());

    do
    {
        m_z_stream.avail_out = static_cast<uInt>(out.size());
        m_z_stream.next_out = reinterpret_cast<uint8_t*>(out.data());

        int res = deflate(&m_z_stream, flush ? Z_FINISH : Z_NO_FLUSH);
        if (res < 0)
            return false;

        size_t output_bytes = out.size() - static_cast<size_t>(m_z_stream.avail_out);
        m_dst.write(out.data(), output_bytes);
        if (!m_dst.good())
            return false;
    } while (m_z_stream.avail_out == 0);

    m_buffer.clear();

    return true;
}

int gzip_ostreambuf::overflow(int c)
{
    if (c == std::char_traits<char>::eof())
        return c;

    if (c > 0xff)
        throw descriptive_exception("char exeeded");

    m_buffer.push_back(static_cast<char>(c));

    if (m_buffer.size() == gzip_ostreambuf::BUFFER_SIZE && !WriteOutput(false))
            throw descriptive_exception("Gzip deflation error.");

    return std::char_traits<char>::to_int_type(static_cast<char>(c));
}

int gzip_ostreambuf::sync()
{
    return WriteOutput(true) ? 0 : -1;
}

} // namespace core