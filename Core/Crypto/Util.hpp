#pragma once

#include "Types.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace core {

template<typename T>
void CalcSHA256(sha256_t& out, const T& in)
{
	EVP_Digest(in.data(), in.size(), out.data(), nullptr, EVP_sha256(), nullptr);
}

template<typename T1, typename T2>
void CalcSHA256(sha256_t& out, const T1& in1, const T2& in2)
{
	auto ctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
	EVP_DigestUpdate(ctx, in1.data(), in1.size());
	EVP_DigestUpdate(ctx, in2.data(), in2.size());
	EVP_DigestFinal_ex(ctx, out.data(), nullptr);
	EVP_MD_CTX_free(ctx);
}

template<typename T>
void RandBytes(T& in)
{
	RAND_bytes(in.data(), static_cast<int>(in.size()));
}

} // namespace core