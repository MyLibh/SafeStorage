#pragma once

#include <chrono>
#include <filesystem>

namespace core {

class Settings final
{
private:
	static std::filesystem::path path;

public:
	static bool Load();
	static bool Save();

	static struct Startup
	{
		bool start_only_one_instance{};
		bool launch_at_system_startup{};
		bool minimize_at_startup{};
		bool minimize_after_unlocking{};
		bool remember_databases{};
	} startup;

	static struct Timeouts
	{
		std::chrono::seconds clear_clipboard{};
		std::chrono::seconds lock_db{};
	} timeouts;

	static struct Security
	{

	} security;
};

} // namespace core