// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Obfuscator.hpp"

namespace core {
    void Obfuscator::FillBuffer()
    {
        static constexpr std::array<uint8_t, 32> kZeroBlock = { 0 };

        m_buffer = m_cipher->Encrypt(kZeroBlock);
        m_buffer_pos = 0;
    }

    std::string Obfuscator::Process(const std::string& data)
    {
        if (!m_cipher)
            return data;

        std::string obfuscated_data;
        obfuscated_data.resize(data.size());

        for (size_t i{}; i < data.size(); ++i) {
            if (m_buffer_pos == m_buffer.size())
                FillBuffer();

            obfuscated_data[i] = data[i] ^ m_buffer[m_buffer_pos++];
        }

        return obfuscated_data;
    }
} // namespace core