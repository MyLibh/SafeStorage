#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <vector>

namespace core {

enum class HeaderType : uint8_t
{
	EndOfHeader,         // 0 bytes
	Comment,
	CipherID,            // 1 byte
	CompressionFlags,    // 1 bytes
	MasterSeed,          // 16 bytes
	TransformSeed,       // 32 bytes
	TransformRounds,     // 8 bytes
	EncryptionIV,        // 16 bytes
	ProtectedStreamKey,
	StreamStartBytes,    // 32 bytes
	InnerRandomStreamID, // 1 bytes
	LastLayer,           // 1 byte
};

struct Header
{
	HeaderType type;
	uint16_t length;
	std::vector<uint8_t> value;

	Header()
		: type(HeaderType::EndOfHeader)
		, length()
	{}

	template<typename T>
	Header(const HeaderType type, const T& val)
		: type(type)
		, length(sizeof(T))
	{
		if (const auto ptr = reinterpret_cast<const uint8_t*>(&val); length)
		{
			value.resize(length);
			std::copy(ptr, ptr + length, value.begin());
		}
	}

	template<size_t N>
	Header(const HeaderType type, const std::array<uint8_t, N>& array)
		: type(type)
		, length(static_cast<uint16_t>(array.size()))
		, value(array.begin(), array.end())
	{
	}

	bool Read(std::istream& input);
	bool Write(std::ostream& output) const;

	bool operator<(const Header& other) const { return static_cast<uint8_t>(type) < static_cast<uint8_t>(other.type); }
};

} // namespace core