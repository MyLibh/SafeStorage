// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Layer.hpp"
#include "Header.hpp"
#include "../Crypto/AES.hpp"
#include "../Crypto/ChaCha20.hpp"
#include "../Crypto/Grasshopper.hpp"
#include "../Crypto/Util.hpp"

#include <algorithm>
#include <cassert>
#include <sstream>

namespace core {

Layer::Layer(std::string&& password,
	const CipherId cipher_id,
	const uint64_t transform_rounds,
	const CompressionType compression_type /*= CompressionType::None*/,
	const RandomStreamId random_stream /*= RandomStreamId::None*/,
	bool is_last /*= false*/)
	: key(std::move(password))
	, cipher_id(cipher_id)
	, transform_rounds(transform_rounds)
	, compression_type(compression_type)
	, inner_random_stream_id(random_stream)
	, is_last(is_last)
{
	ExplicitBZero(password.data(), password.length());

	RandBytes(master_seed);
	RandBytes(iv);
	RandBytes(transform_seed);
	RandBytes(inner_random_stream_key);
	RandBytes(stream_start_bytes);
}

sha256_t Layer::MakeFinalInnerRandomStreamKey() const
{
	sha256_t final_inner_random_stream_key;
	CalcSHA256(final_inner_random_stream_key, inner_random_stream_key);

	return final_inner_random_stream_key;
}

std::unique_ptr<Cipher> Layer::GetCipher() const
{
	switch (cipher_id)
	{
	case CipherId::AES_256_GCM:     return std::make_unique<AES>(MakeFinalKey(), iv);
	case CipherId::GRASSHOPPER_CFB: return std::make_unique<Grasshopper>(MakeFinalKey(), iv);
	default: assert(false);         return nullptr;
	}
}

std::unique_ptr<Cipher> Layer::GetInnerRandomStream() const
{
	switch (inner_random_stream_id)
	{
	case RandomStreamId::None:     return nullptr;
	case RandomStreamId::ChaCha20: return std::make_unique<ChaCha20>(MakeFinalInnerRandomStreamKey(), INNER_RANDOM_STREAM_IV);
	default: assert(false);        return nullptr;
	}
}

void Layer::ReadHeaders(std::istream& input)
{
	while (true)
	{
		Header header;
		if (!header.Read(input))
			break;
		else
			Update(header);
	}

	headers_hash = ComputeHeadersHash();
}

void Layer::WriteHeaders(std::ostream& output) const
{
	Header(HeaderType::CipherID, static_cast<uint8_t>(cipher_id)).Write(output);
	Header(HeaderType::CompressionFlags, static_cast<uint8_t>(compression_type)).Write(output);
	Header(HeaderType::MasterSeed, master_seed).Write(output);
	Header(HeaderType::TransformSeed, transform_seed).Write(output);
	Header(HeaderType::TransformRounds, transform_rounds).Write(output);
	Header(HeaderType::EncryptionIV, iv).Write(output);
	Header(HeaderType::StreamStartBytes, stream_start_bytes).Write(output);
	Header(HeaderType::InnerRandomStreamID, static_cast<uint32_t>(inner_random_stream_id)).Write(output);
	Header(HeaderType::LastLayer, is_last).Write(output);
	Header().Write(output);
}

void Layer::Encrypt(std::stringstream& dst) const
{
	std::stringstream content_stream;
	WriteHeaders(content_stream);

	auto cipher = GetCipher();
	if (!cipher)
		return;

	auto add_val = [&](const auto& val)
	{
		const auto enc_val = cipher->Encrypt(val);
		content_stream.write(reinterpret_cast<const char*>(enc_val.data()), enc_val.size());
	};

	add_val(ComputeHeadersHash());
	add_val(stream_start_bytes);

	cipher->Encrypt(dst, content_stream);
	dst = std::move(content_stream);
}

bool Layer::Decrypt(std::stringstream& src)
{
	auto cipher = GetCipher();
	if (!cipher)
		return false;

	const auto pos_before_dec = src.tellg();
	std::stringstream content;
	cipher->Decrypt(src, content);

	auto check_val = [&](auto& val, const auto& val_to_check)
	{
		content.read(reinterpret_cast<char*>(val.data()), val.size());
		if (!content.good() || val != val_to_check)
		{
			if (src.eof())
				src.clear();
			src.seekg(pos_before_dec);
			return false;
		}

		return true;
	};

	if (sha256_t hash; !check_val(hash, headers_hash))
		return false;

	if (std::array<uint8_t, 32> start_bytes; !check_val(start_bytes, stream_start_bytes))
		return false;

	src = std::move(content);

	return true;
}

sha256_t Layer::MakeFinalKey() const
{
	sha256_t final_key;
	CalcSHA256(final_key, master_seed, key.Transform(transform_seed, transform_rounds));

	return final_key;
}

void Layer::Update(const Header& header)
{
	switch (header.type)
	{
	case HeaderType::EndOfHeader:
	case HeaderType::Comment:
		break;
	case HeaderType::CipherID:
		cipher_id = static_cast<CipherId>(header.value.front());
		break;
	case HeaderType::CompressionFlags:
		compression_type = static_cast<CompressionType>(header.value.front());
		break;
	case HeaderType::MasterSeed:
		std::copy_n(header.value.begin(), header.length, master_seed.begin());
		break;
	case HeaderType::TransformSeed:
		std::copy_n(header.value.begin(), header.length, transform_seed.begin());
		break;
	case HeaderType::TransformRounds:
		transform_rounds = *reinterpret_cast<const uint64_t*>(header.value.data());
		break;
	case HeaderType::EncryptionIV:
		std::copy_n(header.value.begin(), header.length, iv.begin());
		break;
	case HeaderType::ProtectedStreamKey:
		std::copy_n(header.value.begin(), header.length, inner_random_stream_key.begin());
		break;
	case HeaderType::StreamStartBytes:
		std::copy_n(header.value.begin(), header.length, stream_start_bytes.begin());
		break;
	case HeaderType::InnerRandomStreamID:
		inner_random_stream_id = *reinterpret_cast<const RandomStreamId*>(header.value.data());
		break;
	case HeaderType::LastLayer:
		is_last = *header.value.data();
		break;
	default:
		assert(false);
		break;
	}
}

sha256_t Layer::ComputeHeadersHash() const
{
	std::stringstream stream;
	WriteHeaders(stream);
		
	sha256_t hash;
	CalcSHA256(hash, stream.str());

	return hash;
}

} // namespace core