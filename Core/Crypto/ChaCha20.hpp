#pragma once

#include "Cipher.hpp"

#include <openssl/types.h>

namespace core {

class ChaCha20 final : public Cipher
{
public:
	ChaCha20(const std::array<uint8_t, 32>& key, const iv_t& iv);
	virtual ~ChaCha20() override;

	void Encrypt(std::istream& src, std::ostream& dst) override;
	void Decrypt(std::istream& src, std::ostream& dst) override;

	std::array<uint8_t, 32> Encrypt(const std::array<uint8_t, 32>& src_block) override;

protected:
	EVP_CIPHER_CTX* m_enc_key;
	EVP_CIPHER_CTX* m_dec_key;
};

} // namespace core