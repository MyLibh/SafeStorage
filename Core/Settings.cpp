// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Settings.hpp"

#include <fstream>

#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

namespace core {

std::filesystem::path Settings::path = (QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation) + QDir::separator() +
	QString("ssdb_settings.json")).toStdString();

Settings::Startup Settings::startup{};
Settings::Timeouts Settings::timeouts{};
Settings::Security Settings::security{};

bool Settings::Load()
{
	if (!std::filesystem::exists(Settings::path))
		return false;

	std::ifstream in(path);
	if (!in.is_open())
		return false;

	QJsonDocument settings;
	{
		std::string str(std::istreambuf_iterator<char>(in), {});
		settings = QJsonDocument::fromJson(str.c_str());
	}

	{ // Startup
		auto& obj = settings["startup"];
		Settings::startup.start_only_one_instance = obj["start_only_one_instance"].toBool();
		Settings::startup.launch_at_system_startup = obj["launch_at_system_startup"].toBool();
		Settings::startup.minimize_at_startup = obj["minimize_at_startup"].toBool();
		Settings::startup.minimize_after_unlocking = obj["minimize_after_unlocking"].toBool();
		Settings::startup.remember_databases = obj["remember_databases"].toBool();
	}

	{ // Timeouts
		auto& obj = settings["timeouts"];
		Settings::timeouts.clear_clipboard = std::chrono::seconds(obj["clear_clipboard"].toInt());
		Settings::timeouts.lock_db = std::chrono::seconds(obj["lock_db"].toInt());
	}

	{ // Security
		// auto& obj = settings["security"];
	}

	return true;
}

bool Settings::Save()
{
	QJsonObject settings;
	{ // Startup
		QJsonObject obj;
		obj["start_only_one_instance"] = Settings::startup.start_only_one_instance;
		obj["launch_at_system_startup"] = Settings::startup.launch_at_system_startup;
		obj["minimize_at_startup"] = Settings::startup.minimize_at_startup;
		obj["minimize_after_unlocking"] = Settings::startup.minimize_after_unlocking;
		obj["remember_databases"] = Settings::startup.remember_databases;
		settings.insert("startup", obj);
	}

	{ // Timeouts
		QJsonObject obj;
		obj["clear_clipboard"] = static_cast<qint64>(Settings::timeouts.clear_clipboard.count());
		obj["lock_db"] = static_cast<qint64>(Settings::timeouts.lock_db.count());
		settings.insert("timeouts", obj);
	}

	{ // Security
		QJsonObject obj;
		obj.insert("security", obj);
	}

	QJsonDocument doc(settings);
	std::ofstream out(Settings::path);
	if (!out.is_open())
		return false;

	out << QString(doc.toJson(QJsonDocument::Compact)).toStdString();

	return false;
}

} // namespace core
