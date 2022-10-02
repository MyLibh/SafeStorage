#pragma once

#include "../Constants.hpp"
#include "../Crypto/Util.hpp"
#include "../Key.hpp"

#include <iosfwd>
#include <memory>

namespace core {

class Cipher;
struct Header;

struct Layer
{
	Key key{};
	CipherId cipher_id{};
	std::array<uint8_t, 16> master_seed{};
	std::array<uint8_t, 16> iv{};
	std::array<uint8_t, 32> transform_seed{};
	std::array<uint8_t, 32> inner_random_stream_key{};
	std::array<uint8_t, 32> stream_start_bytes{};
	uint64_t transform_rounds{};
	CompressionType compression_type{};
	RandomStreamId inner_random_stream_id{};
	sha256_t headers_hash{};
	bool is_last{};

public:
	Layer() noexcept = default;
	Layer(std::string&& password,
		const CipherId cipher,
		const uint64_t transform_rounds,
		const CompressionType compression_type = CompressionType::None,
		const RandomStreamId random_stream = RandomStreamId::None,
		bool is_last = false);

	[[nodiscard]] std::unique_ptr<Cipher> GetCipher() const;
	[[nodiscard]] std::unique_ptr<Cipher> GetInnerRandomStream() const;

	void ReadHeaders(std::istream& input);
	void WriteHeaders(std::ostream& output) const;

	void Encrypt(std::stringstream& dst) const;
	bool Decrypt(std::stringstream& src);

	[[nodiscard]] sha256_t ComputeHeadersHash() const;

private:
	[[nodiscard]] sha256_t MakeFinalKey() const;
	[[nodiscard]] sha256_t MakeFinalInnerRandomStreamKey() const;
	void Update(const Header& header);
};

} // namespace core