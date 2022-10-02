#pragma once

#include "../Crypto/Cipher.hpp"

#include <array>
#include <memory>
#include <string>

namespace core {

class Obfuscator
{
public:
	explicit Obfuscator(std::unique_ptr<Cipher>&& cipher)
		: m_cipher(std::move(cipher))
		, m_buffer()
		, m_buffer_pos()
	{}

	std::string Process(const std::string& data);

private:
	void FillBuffer();

private:
	std::unique_ptr<Cipher> m_cipher;

	std::array<uint8_t, 32> m_buffer;
	size_t m_buffer_pos;
};

} // namespace core