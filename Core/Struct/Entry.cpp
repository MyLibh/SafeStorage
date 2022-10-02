// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Entry.hpp"

#include "Obfuscator.hpp"
#include "../PasswordChecker.hpp"
#include "../Util.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

namespace core {

inline static constexpr auto CREATION_TIME_FIELD{ "creation_time" };
inline static constexpr auto LAST_MODIFICATION_TIME_FIELD{ "last_modification_time" };
inline static constexpr auto LAST_ACCESS_TIME_FIELD{ "last_access_time" };
inline static constexpr auto TIMES_FIELD{ "times" };

inline static constexpr auto UUID_FIELD{ "uuid" };
inline static constexpr auto TITLE_FIELD{ "title" };
inline static constexpr auto URL_FIELD{ "url" };
inline static constexpr auto USERNAME_FIELD{ "username" };
inline static constexpr auto PASSWORD_FIELD{ "password" };
inline static constexpr auto NOTE_FIELD{ "note" };
inline static constexpr auto PASSWORDS_FIELD{ "passwords" };

TimesEntry::TimesEntry(const QJsonValue& value, Obfuscator& /*obfuscator*/)
	: creation_time(value[CREATION_TIME_FIELD].toString().toStdString())
	, last_modification_time(value[LAST_MODIFICATION_TIME_FIELD].toString().toStdString())
	, last_access_time(value[LAST_ACCESS_TIME_FIELD].toString().toStdString())
{}

void TimesEntry::ToJson(QJsonObject& obj, Obfuscator& /*obfuscator*/) const
{
	QJsonObject times_entry;
	times_entry.insert(CREATION_TIME_FIELD, creation_time.c_str());
	times_entry.insert(LAST_MODIFICATION_TIME_FIELD, last_modification_time.c_str());
	times_entry.insert(LAST_ACCESS_TIME_FIELD, last_access_time.c_str());

	obj.insert(TIMES_FIELD, times_entry);
}

PasswordEntry::PasswordEntry(const QJsonValue& value, Obfuscator& obfuscator)
	: TimesEntry(value[TIMES_FIELD], obfuscator)
	, uuid(value[UUID_FIELD].toString().toStdString())
	, title(value[TITLE_FIELD].toString().toStdString())
	, url(value[URL_FIELD].toString().toStdString())
	, username(value[USERNAME_FIELD].toString().toStdString())
	, password(obfuscator.Process(value[PASSWORD_FIELD].toString().toStdString()))
	, note(value[NOTE_FIELD].toString().toStdString())
{}

void PasswordEntry::ToJson(QJsonObject& obj, Obfuscator& obfuscator) const
{
	TimesEntry::ToJson(obj, obfuscator);

	obj.insert(UUID_FIELD, uuid.c_str());
	obj.insert(TITLE_FIELD, title.c_str());
	obj.insert(URL_FIELD, url.c_str());
	obj.insert(USERNAME_FIELD, username.c_str());
	obj.insert(PASSWORD_FIELD, obfuscator.Process(password).c_str());
	obj.insert(NOTE_FIELD, note.c_str());
}

PasswordsEntry::PasswordsEntry(const QJsonValue& value, Obfuscator& obfuscator)
{
	for (const auto& el : value.toArray())
		m_entries.emplace_back(el, obfuscator);
}

void PasswordsEntry::ToJson(QJsonObject& obj, Obfuscator& obfuscator) const
{
	QJsonArray arr;
	for (const auto& el : m_entries)
	{
		QJsonObject entry;
		el.ToJson(entry, obfuscator);
		arr << entry;
	}

	obj.insert(PASSWORDS_FIELD, arr);
}

void PasswordsEntry::AddEntry(PasswordEntry&& entry)
{
	std::string uuid;
	while (true)
	{
		uuid = gen_uuid();

		if (auto it = std::find_if(std::begin(m_entries), std::end(m_entries), [&](const auto& entry) { return entry.uuid == uuid; }); it == std::end(m_entries))
			break;
	}

	entry.uuid = std::move(uuid);
	entry.creation_time = to_string(std::chrono::system_clock::now());

	m_entries.emplace_back(std::move(entry));
}

void PasswordsEntry::UpdateEntry(const size_t idx, update_cb_t&& cb)
{
	if (idx >= m_entries.size())
		return;

	if (cb(m_entries[idx]))
		m_entries[idx].last_modification_time = to_string(std::chrono::system_clock::now());
}

void PasswordsEntry::RemoveEntry(const std::string_view uuid)
{
	if (auto it = std::find_if(std::begin(m_entries), std::end(m_entries), [&](const auto& entry) { return entry.uuid == uuid; }); it != std::end(m_entries))
		m_entries.erase(it);
}

void PasswordsEntry::RemoveEntry(const size_t idx)
{
	m_entries.erase(std::next(m_entries.begin(), idx));
}

std::vector<int> PasswordsEntry::GetLeakedPasswords() const
{
	std::vector<int> vec;
	for (size_t i{}; i < m_entries.size(); ++i)
		if (m_entries[i].leaked)
			vec.push_back(static_cast<int>(i));

	return vec;
}

std::vector<int> PasswordsEntry::CheckPasswords()
{
	std::vector<int> vec;
	for (size_t i{}; i < m_entries.size(); ++i)
		if (!m_entries[i].leaked && !m_entries[i].password.empty() && !PasswordChecker::Check(m_entries[i].password))
		{
			m_entries[i].leaked = true;
			vec.push_back(static_cast<int>(i));
		}

	return vec;
}

} // namespace core