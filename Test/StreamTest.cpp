#include "Stream/GZip.hpp"
#include "Stream/Hashed.hpp"

#include <gtest/gtest.h>

using namespace core;

template<typename OStreamBuf, typename IStreamBuf>
void TestStreamBuf()
{
	std::stringstream stream;

	OStreamBuf osreambuf(stream);
	std::ostream ostream(&osreambuf);
	ostream << "text";
	ostream.flush();

	IStreamBuf istreambuf(stream);
	std::istream istream(&istreambuf);
	std::string str(std::istreambuf_iterator<char>(istream), {});

	EXPECT_STREQ(str.c_str(), "text");
}

TEST(StreamTest, TestGZipEncDec)
{
	TestStreamBuf<gzip_ostreambuf, gzip_istreambuf>();
}

TEST(StreamTest, TestHashedEncDec)
{
	TestStreamBuf<hashed_ostreambuf, hashed_istreambuf>();
}