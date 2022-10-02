#pragma once

#include <QStyledItemDelegate>

class RowColorDelegate : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QVector<int> rows() const { return m_rows; }

    void SetRows(QVector<int>&& rows) { m_rows = std::move(rows); }

protected:
    void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override
    {
        QStyledItemDelegate::initStyleOption(option, index);
        QVariant value = index.data();

        if (m_rows.contains(index.row()))
            option->backgroundBrush = QBrush(QColor("red"));
    }

private:
    QVector<int> m_rows;
};