// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "PasswordsModel.hpp"

#include <cassert>

#include <QColor>

int PasswordsModel::rowCount(const QModelIndex& /*idx*/) const
{
    if (const auto lock = m_data.lock())
        return static_cast<int>(lock->size());

    return 0;
}

QVariant PasswordsModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return {};

    auto lock = m_data.lock();
    if (!lock)
        return {};

    const auto& entry = (*lock)[static_cast<size_t>(index.row())];
    switch (index.column())
    {
    case Index::UUID:     return entry.uuid.c_str();
    case Index::Title:    return entry.title.c_str();
    case Index::URL:      return entry.url.c_str();
    case Index::Username: return entry.username.c_str();
    case Index::Password: return m_show_passwords ? entry.password.c_str() : PasswordsModel::HIDE_PASSWORD_STRING;
    case Index::Note:     return entry.note.c_str();
    default:
        assert(false);
        return {};
    };
}

QVariant PasswordsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole || section < 0)
        return {};

    switch (section)
    {
    case Index::UUID:       return tr("UUID");
    case Index::Title:      return tr("Title");
    case Index::URL:        return tr("URL");
    case Index::Username:   return tr("Username");
    case Index::Password:   return tr("Password");
    case Index::Note:       return tr("Note");
    default: assert(false); return {};
    }
}

bool PasswordsModel::setData(const QModelIndex& index, const QVariant& value, int role /*= Qt::EditRole*/)
{
    if ((role != Qt::EditRole && role != Qt::BackgroundRole) || index.column() == Index::UUID)
        return false;

    if (auto lock = m_data.lock())
        lock->UpdateEntry(static_cast<size_t>(index.row()), [col = index.column(), val = value.toString().toStdString()](auto& entry) {
        switch (col)
        {
        case Index::Title:    entry.title = std::move(val); return true;
        case Index::URL:      entry.url = std::move(val); return true;
        case Index::Username: entry.username = std::move(val); return true;
        case Index::Password: entry.password = std::move(val); return true;
        case Index::Note:     entry.note = std::move(val); return true;
        default:
            assert(false);
            return false;
        }
    });

    emit dataChanged(index, index);

    return true;
}

Qt::ItemFlags PasswordsModel::flags(const QModelIndex& idx) const
{
    if (idx.column() == Index::UUID)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
}

void PasswordsModel::AddEntry()
{
    if (auto lock = m_data.lock())
        lock->AddEntry({});

    emit dataChanged(index(0, 0), index(rowCount({}) - 1, Index::Total - 1));
    emit layoutChanged();
}

void PasswordsModel::RemoveEntry(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;

    if (auto lock = m_data.lock())
        lock->RemoveEntry(static_cast<size_t>(idx.row()));

    removeRow(idx.row());
}

QVector<int> PasswordsModel::CheckPasswords() const
{
    const auto lock = m_data.lock();
    if (!lock)
        return {};

    const auto vec = lock->CheckPasswords();
    return QVector(vec.begin(), vec.end());
}

QVector<int> PasswordsModel::GetLeakedPasswords() const
{
    const auto lock = m_data.lock();
    if (!lock)
        return {};

    const auto vec = lock->GetLeakedPasswords();
    return QVector(vec.begin(), vec.end());
}
