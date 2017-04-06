#pragma once

#include <QStyledItemDelegate>
#include <QWidget>


class QLogHistoryItemDelegate : public QStyledItemDelegate
{
public:
    QLogHistoryItemDelegate(QWidget *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
