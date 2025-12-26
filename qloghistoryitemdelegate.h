#pragma once

#include <QStyledItemDelegate>
#include <QWidget>
#include <QBrush>
#include <QList>


class QLogHistoryItemDelegate : public QStyledItemDelegate
{
public:
    explicit QLogHistoryItemDelegate(QWidget *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};
