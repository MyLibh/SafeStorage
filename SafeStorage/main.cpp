// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "MainWindow.hpp"
#include "Settings.hpp"
#include "PasswordChecker.hpp"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QLockFile>
#include <QMessageBox>
#include <QObject>
#include <QTextStream>

auto main(int argc, char* argv[]) -> signed try
{
    QApplication app(argc, argv);
    if (QFile f(":qdarkstyle/dark/darkstyle.qss"); f.exists())
    {
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream ts(&f);
        app.setStyleSheet(ts.readAll());
    }

    if (auto wordlist = std::make_unique<QFile>(":wordlist/rockyou.txt"); wordlist && wordlist->exists())
    {
        wordlist->open(QFile::ReadOnly | QFile::Text);
        core::PasswordChecker::SetWordlistFile(std::move(wordlist));
    }

    core::Settings::Load();

    {
        QFileInfo fileInfo(QCoreApplication::applicationFilePath());
        const auto link_filepath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QDir::separator() +
            "Startup" + QDir::separator() +
            fileInfo.completeBaseName() + ".lnk";
        if (core::Settings::startup.launch_at_system_startup)
            QFile::link(QCoreApplication::applicationFilePath(), link_filepath);
        else
            QFile(link_filepath).remove();
    }

    QLockFile lockFile(QDir::temp().absoluteFilePath("SafeStorage.lock"));
    if (core::Settings::startup.start_only_one_instance && !lockFile.tryLock(100))
    {
        QMessageBox::warning(nullptr, QObject::tr("Error"), QObject::tr("Only one instance can run at once"), QMessageBox::StandardButton::Ok);
        return 1;
    }

    MainWindow window;
    if (core::Settings::startup.minimize_at_startup)
        window.showMinimized();

    window.show();
    return app.exec();
}
catch (...)
{
    return 1;
}