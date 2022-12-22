#include "Crypto/AES.hpp"
#include "Crypto/ChaCha20.hpp"
#include "Crypto/Obfuscator.hpp"
#include "Struct/Layer.hpp"

#include <memory>
#include <string>

#include <gtest/gtest.h>

using namespace core;

TEST(CryptoTest, TestAesEncDec)
{
	core::AES aes({}, {});
	std::stringstream in("plaintext");
	std::stringstream out;
	aes.Encrypt(in, out);
	std::stringstream out2;
	aes.Decrypt(out, out2);
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