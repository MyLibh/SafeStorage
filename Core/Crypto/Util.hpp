#pragma once

#include "Types.hpp"

#include <openssl/evp.h>
#include <openssl/rand.h>

namespace core {

template<typename T>
inline void CalcSHA256(sha256_t& out, const T& in)
{
	EVP_Digest(in.data(), in.size(), out.data(), nullptr, EVP_sha256(), nullptr);
}

template<typename T1, typename T2>
inline void CalcSHA256(sha256_t& out, const T1& in1, const T2& in2)
{
	auto ctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
	EVP_DigestUpdate(ctx, in1.data(), in1.size());
	EVP_DigestUpdate(ctx, in2.data(), in2.size());
	EVP_DigestFinal_ex(ctx, out.data(), nullptr);
	EVP_MD_CTX_free(ctx);
}

template<typename T>
inline void RandBytes(T& in)
{
	RAND_bytes(in.data(), static_cast<int>(in.size()));
}

inline void ExplicitBZero(void* p, size_t n)
{
	auto pp = static_cast<volatile char*>(p);
	if constexpr (sizeof(void*) == 8)
		for (; n >= 8; pp += 8, n -= 8)
			*reinterpret_cast<volatile long long*>(pp) = 0;
	else
		for (; n >= 4; pp += 4, n -= 4)
			*reinterpret_cast<volatile int*>(pp) = 0;
	for (; n >= 1; ++pp, --n)
		*pp = 0;
}

} // namespace core