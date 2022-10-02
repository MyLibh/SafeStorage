#pragma once

#include <array>
#include <iosfwd>

namespace core {

class Cipher
{
public:
	inline static constexpr auto IV_SIZE{ 16 };

	using iv_t = std::array<uint8_t, IV_SIZE>;

public:
	Cipher(const iv_t& iv) : m_iv(iv) {}
	virtual ~Cipher() = default;

	const iv_t& GetIV() const { return m_iv; }

	virtual void Encrypt(std::istream& src, std::ostream& dst) = 0;
	virtual void Decrypt(std::istream& src, std::ostream& dst) = 0;

	virtual std::array<uint8_t, 32> Encrypt(const std::array<uint8_t, 32>& src_block) = 0;

protected:
	const iv_t m_iv;
};

} // namespace core