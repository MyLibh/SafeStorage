#pragma once

#include "Cipher.hpp"

namespace core {

class Grasshopper final : public Cipher
{
public:
	Grasshopper(const std::array<uint8_t, 32>& key, const iv_t& iv);
	virtual ~Grasshopper() override = default;

	void Encrypt(std::istream& src, std::ostream& dst) override;
	void Decrypt(std::istream& src, std::ostream& dst) override;

	std::array<uint8_t, 32> Encrypt(const std::array<uint8_t, 32>& src_block) override;

protected:
	std::array<std::array<uint8_t, 16>, 10> m_keys;
	std::array<uint8_t, 16> m_prev;
};

} // namespace core