#pragma once

#include <string>

namespace core {

class descriptive_exception : public std::exception
{
public:
	descriptive_exception(std::string const& message)
		: m_msg(message)
	{}

	virtual char const* what() const noexcept { return m_msg.c_str(); }

private:
	std::string m_msg;
};

} // namespace core