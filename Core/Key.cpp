// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Key.hpp"
#include "Crypto/AES.hpp"
#include "Crypto/Util.hpp"

namespace core {

sha256_t Key::Transform(const sha256_t& seed, const uint64_t rounds) const
{
	AES cipher(seed, {}, AESType::ECB_256);

	auto transformed_key = m_key.Resolve();
	for (uint64_t i{}; i < rounds; ++i)
		transformed_key = cipher.Encrypt(transformed_key);

	sha256_t result;
	CalcSHA256(result, transformed_key);

	return result;
}

void Key::SetPassword(std::string&& password)
{
	CalcSHA256(m_key.key_password, password);
}

sha256_t Key::CompositeKey::Resolve() const
{
	sha256_t key;
	CalcSHA256(key, key_password, key_keyfile);

	return key;
}

} // namespace core