// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "PasswordChecker.hpp"

#include <QFile>
#include <QTextStream>

namespace core {

std::unique_ptr<QFile> PasswordChecker::wordlist;

bool PasswordChecker::Check(const std::string_view password)
{
    if (!PasswordChecker::wordlist)
        throw "no wordlist set";

    wordlist->seek(0);
    QTextStream in(PasswordChecker::wordlist.get());
    QString line;
    do {
        line = in.readLine();
        if (line.contains(password.data(), Qt::CaseSensitive))
            return false;

    } while (!line.isNull());

    return true;
}

} // namespace core