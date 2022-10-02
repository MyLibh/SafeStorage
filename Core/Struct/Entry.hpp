#pragma once

#include <chrono>
#include <functional>
#include <string>
#include <vector>

class QJsonObject;
class QJsonValue;

namespace core {

class Obfuscator;

struct TimesEntry
{
	std::string/*std::chrono::steady_clock::time_point*/ creation_time;
	std::string/*std::chrono::steady_clock::time_point*/ last_modification_time;
	std::string/*std::chrono::steady_clock::time_point*/ last_access_time;

	TimesEntry() noexcept = default;
	TimesEntry(const QJsonValue& value, Obfuscator& obfuscator);

	virtual void ToJson(QJsonObject& obj, Obfuscator& obfuscator) const;
};

struct PasswordEntry : TimesEntry
{
	std::string uuid;
	std::string title;
	std::string url;
	std::string username;
	std::string password;
	std::string note;

	bool leaked{};

	PasswordEntry() noexcept = default;
	PasswordEntry(const QJsonValue& value, Obfuscator& obfuscator);

	void ToJson(QJsonObject& obj, Obfuscator& obfuscator) const override;

	bool operator<(const PasswordEntry& other) const { return uuid < other.uuid; }
};

struct PasswordsEntry
{
public:
	using update_cb_t = std::function<bool (PasswordEntry&)>;

	PasswordsEntry() noexcept = default;
	PasswordsEntry(const QJsonValue& value, Obfuscator& obfuscator);

	size_t size() const { return m_entries.size(); }

	const PasswordEntry& operator[](const size_t idx) const { return *std::next(m_entries.begin(), idx); }

	void ToJson(QJsonObject& obj, Obfuscator& obfuscator) const;

	void AddEntry(PasswordEntry&& entry);
	void AddEntry() { AddEntry({}); }
	void UpdateEntry(const size_t idx, update_cb_t&& cb);
	void RemoveEntry(const std::string_view uuid);
	void RemoveEntry(const size_t idx);

	std::vector<int> GetLeakedPasswords() const;
	std::vector<int> CheckPasswords();

private:
	std::vector<PasswordEntry> m_entries;
};

} // namespace core