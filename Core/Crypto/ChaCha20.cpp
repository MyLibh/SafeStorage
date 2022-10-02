// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "ChaCha20.hpp"

#include <openssl/evp.h>

#include <cassert>
#include <iostream>

namespace core {

ChaCha20::ChaCha20(const std::array<uint8_t, 32>& key, const iv_t& iv)
	: Cipher(iv)
	, m_enc_key(EVP_CIPHER_CTX_new())
	, m_dec_key(EVP_CIPHER_CTX_new())
{
	EVP_EncryptInit(m_enc_key, EVP_chacha20(), key.data(), iv.data());
	EVP_DecryptInit(m_dec_key, EVP_chacha20(), key.data(), iv.data());
}

ChaCha20::~ChaCha20()
{
	EVP_CIPHER_CTX_free(m_enc_key);
	EVP_CIPHER_CTX_free(m_dec_key);
}

void ChaCha20::Encrypt(std::istream& /*src*/, std::ostream& /*dst*/)
{
	assert(false);
}

void ChaCha20::Decrypt(std::istream& /*src*/, std::ostream& /*dst*/)
{
	assert(false);
}

std::array<uint8_t, 32> ChaCha20::Encrypt(const std::array<uint8_t, 32>& src_block)
{
	std::array<uint8_t, 32> out;

	[[maybe_unused]] int outl{};
	EVP_EncryptUpdate(m_enc_key, out.data(), &outl, src_block.data(), static_cast<int>(src_block.size()));
	assert(static_cast<size_t>(outl) == out.size());

	return out;
}

} // namespace core