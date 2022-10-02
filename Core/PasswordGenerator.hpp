#pragma once

#include <bitset>
#include <cmath>
#include <string>

namespace core {

class PasswordGenerator
{
public:
	enum class Flag : size_t
	{
		Capitals,    // A-Z
		Lowercase,   // a-z
		Digits,      // 0-9
		Special1,    // # $ % & @ ^ ` ~
		Special2,    // . , : ;
		Special3,    // " '
		Special4,    // / | _ -
		Special5,    // < * + ! ? =
		Parentheses, // { [ ( ) ] }

		Total
	};

	static PasswordGenerator& GetInstance()
	{
		static PasswordGenerator instance;
		return instance;
	}

	void SetLength(const uint8_t length) noexcept { m_length = length; }
	void SetUseFlag(const Flag flag, const bool use) { m_flags.set(static_cast<size_t>(flag), use); }

	[[nodiscard]] bool IsAnyFlagSet() const noexcept { return m_flags.any(); }

	[[nodiscard]] std::string GeneratePassword() const;

	[[nodiscard]] uint8_t GetPoolSize() const;
	[[nodiscard]] float GetEntropy() const { return m_length * std::log2f(GetPoolSize()); }

private:
	uint8_t m_length{16};
	std::bitset<static_cast<size_t>(Flag::Total)> m_flags;
};

} // namespace core