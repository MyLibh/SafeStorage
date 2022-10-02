#pragma once

#include <array>
#include <cstdint>

namespace core {

enum class CipherId : uint8_t
{
	None,
	AES_256_GCM,
};

enum class KDF : uint8_t
{
	None,
	AES_KDF,
};

enum class CompressionType : uint8_t
{
	None,
	GZip,
};

enum class RandomStreamId : uint32_t
{
	None,
	ChaCha20,
};

static constexpr uint32_t SSDB_FILE_SIGNATURE = 0x12345678;

constexpr std::array<uint8_t, 16> INNER_RANDOM_STREAM_IV{
	0xe8, 0x30, 0x09, 0x4b, 0x97, 0x20, 0x5d, 0x2a,
	0xe8, 0x30, 0x09, 0x4b, 0x97, 0x20, 0x5d, 0x2a
};

} // namespace core