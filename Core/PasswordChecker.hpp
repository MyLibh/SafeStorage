#pragma once

#include <memory>
#include <string_view>

#include <QFile>

namespace core {

class PasswordChecker
{
public:
	static bool Check(const std::string_view password);

	static void SetWordlistFile(std::unique_ptr<QFile>&& file) noexcept { wordlist = std::move(file); }

private:
	static std::unique_ptr<QFile> wordlist;
};

} // namespace core