#include "Crypto/AES.hpp"
#include "Crypto/ChaCha20.hpp"
#include "Crypto/Grasshopper.cpp"
#include "Crypto/Obfuscator.hpp"
#include "Struct/Layer.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

using namespace core;

TEST(CryptoTest, TestAesEncDec)
{
	AES aes({}, {});
	std::stringstream in("plaintext");
	std::stringstream out;
	aes.Encrypt(in, out);
	std::stringstream out2;
	aes.Decrypt(out, out2);
	EXPECT_STREQ(in.str().c_str(), out2.str().c_str());
}

TEST(CryptoTest, TestGrasshoperEncDecBlock)
{
	std::array<std::array<uint8_t, 16>, 10> keys;
	detail::ExpandKey({}, keys);

	std::array<uint8_t, 16> src, dst, dst2;
	detail::Encrypt(src, dst, keys);

	detail::Decrypt(dst, dst2, keys);
	EXPECT_EQ(src, dst2);
}

TEST(CryptoTest, TestGrasshoperEncDec)
{
	Grasshopper gh({}, {});
	std::stringstream in("plaintext");
	std::stringstream out;
	gh.Encrypt(in, out);
	std::stringstream out2;
	gh.Decrypt(out, out2);
	EXPECT_STREQ(in.str().c_str(), out2.str().c_str());
}

TEST(CryptoTest, TestChaCha20Obfuscation)
{
	std::string plain = "plaintext";
	std::string obfuscated;
	{
		Obfuscator obfuscator(Layer().GetInnerRandomStream());
		obfuscated = obfuscator.Process(plain);
	}
	std::string deobfuscated;
	{
		Obfuscator obfuscator(Layer().GetInnerRandomStream());
		deobfuscated = obfuscator.Process(obfuscated);
	}

	EXPECT_STREQ(plain.c_str(), deobfuscated.c_str());
}