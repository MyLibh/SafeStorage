#pragma once

#include "Struct/Entry.hpp"

#include <QAbstractTableModel>

class PasswordsModel : public QAbstractTableModel
{
private:
    enum Index
    {
        UUID,
        Title,
        URL,
        Username,
        Password,
        Note,

        Total,
    };

public:
    inline static constexpr auto HIDE_PASSWORD_STRING{ "********" };

    PasswordsModel(QObject* parent = nullptr)
        : QAbstractTableModel{ parent }
    {}

    int rowCount(const QModelIndex& /*idx*/) const override;
    int columnCount(const QModelIndex& /*idx*/) const override { return Index::Total; }
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& idx) const override;

    void SetData(std::weak_ptr<core::PasswordsEntry>&& data) noexcept
    {
        m_data = std::move(data);
        emit dataChanged(index(0, 0), index(rowCount({}) - 1, Index::Total - 1));
        emit layoutChanged();
    }

    void AddEntry();
    void RemoveEntry(const QModelIndex& idx);

    void SetShowPasswords(const bool show) noexcept { m_show_passwords = show; }
    bool GetShowPasswords() const noexcept { return m_show_passwords; }

    QVector<int> CheckPasswords() const;
    QVector<int> GetLeakedPasswords() const;

private:
    std::weak_ptr<core::PasswordsEntry> m_data;
    bool m_show_passwords{ false };
};