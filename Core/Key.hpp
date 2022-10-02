#pragma once

#include "Constants.hpp"
#include "Crypto/Types.hpp"

#include <array>
#include <string>

namespace core {

class Key final
{
public:
	Key() = default;
	Key(std::string&& password) { SetPassword(std::move(password)); }

	sha256_t Transform(const sha256_t& seed, const uint64_t rounds) const;

	void SetPassword(std::string&& password);

private:
	struct CompositeKey
	{
		sha256_t key_password{};
		sha256_t key_keyfile{};

		sha256_t Resolve() const;
	} m_key;
};

} // namespace core