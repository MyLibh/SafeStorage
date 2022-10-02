// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "AES.hpp"

#include <openssl/evp.h>
#include <openssl/aes.h>

#include <cassert>
#include <iostream>

namespace core {

AES::AES(const std::array<uint8_t, 32>& key, const iv_t& iv, const AESType type /*= AESType::GCM_256*/)
	: Cipher(iv)
	, m_enc_key(EVP_CIPHER_CTX_new())
	, m_dec_key(EVP_CIPHER_CTX_new())
{
	auto cipher = EVP_aes_256_gcm();
	switch (type)
	{
	case AESType::GCM_256: cipher = EVP_aes_256_gcm(); break;
	case AESType::ECB_256: cipher = EVP_aes_256_ecb(); break;

	default:
		assert(false);
		break;
	}

	EVP_EncryptInit(m_enc_key, cipher, key.data(), iv.data());
	EVP_DecryptInit(m_dec_key, cipher, key.data(), iv.data());
}

AES::~AES()
{
	EVP_CIPHER_CTX_free(m_enc_key);
	EVP_CIPHER_CTX_free(m_dec_key);
}

void AES::Encrypt(std::istream& src, std::ostream& dst)
{
	std::array<uint8_t, 128> src_block, dst_block;

	std::streampos pos = src.tellg();
	src.seekg(0, std::ios::end);
	std::streampos end = src.tellg();
	src.seekg(pos);

	std::streamsize remaining = end - pos;

	while (src.good())
	{
		src.read(reinterpret_cast<char*>(src_block.data()), src_block.size());
		if (src.eof() && src.gcount() == 0)
			break;

		auto read_bytes = src.gcount();
		remaining -= read_bytes;

		int outl{};
		EVP_EncryptUpdate(m_enc_key, dst_block.data(), &outl, src_block.data(), static_cast<int>(read_bytes));

		dst.write(reinterpret_cast<const char*>(dst_block.data()), outl);
	}

	int outl{};
	EVP_EncryptFinal_ex(m_enc_key, dst_block.data(), &outl);
	dst.write(reinterpret_cast<const char*>(dst_block.data()), outl);
}

void AES::Decrypt(std::istream& src, std::ostream& dst)
{
	std::array<uint8_t, 128> src_block, dst_block;

	std::streampos pos = src.tellg();
	src.seekg(0, std::ios::end);
	std::streampos end = src.tellg();
	src.seekg(pos, std::ios::beg);

	std::streamsize remaining = end - pos;

	while (src.good())
	{
		src.read(reinterpret_cast<char*>(src_block.data()), src_block.size());
		if (src.eof() && src.gcount() == 0)
			break;

		auto read_bytes = src.gcount();
		remaining -= read_bytes;

		int outl{};
		EVP_DecryptUpdate(m_dec_key, dst_block.data(), &outl, src_block.data(), static_cast<int>(read_bytes));

		dst.write(reinterpret_cast<const char*>(dst_block.data()), outl);
	}

	int outl{};
	EVP_DecryptFinal_ex(m_dec_key, dst_block.data(), &outl);
	dst.write(reinterpret_cast<const char*>(dst_block.data()), outl);
}

std::array<uint8_t, 32> AES::Encrypt(const std::array<uint8_t, 32>& src_block)
{
	std::array<uint8_t, 32> out;

	[[maybe_unused]] int outl{};
	EVP_EncryptUpdate(m_enc_key, out.data(), &outl, src_block.data(), static_cast<int>(src_block.size()));
	assert(static_cast<size_t>(outl) == out.size());

	return out;
}

} // namespace core