// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Header.hpp"

#include <istream>

namespace core {

bool Header::Read(std::istream& input)
{
	input.read(reinterpret_cast<char*>(&type), sizeof(type));
	input.read(reinterpret_cast<char*>(&length), sizeof(length));

	if (length)
	{
		value.resize(length);
		input.read(reinterpret_cast<char*>(value.data()), length);
	}

	return type != HeaderType::EndOfHeader || input.fail();
}

bool Header::Write(std::ostream& output) const
{
	output.write(reinterpret_cast<const char*>(&type), sizeof(type));
	output.write(reinterpret_cast<const char*>(&length), sizeof(length));

	if (length)
		output.write(reinterpret_cast<const char*>(value.data()), length);

	return output.fail();
}

} // namespace core