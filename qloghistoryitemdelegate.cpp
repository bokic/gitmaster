#include "qloghistoryitemdelegate.h"

#include <QPalette>
#include <QPainter>


QLogHistoryItemDelegate::QLogHistoryItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}

void QLogHistoryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (index.column() == 0)
    {
        auto data = index.data(Qt::UserRole).toList();

        if (data.count() > 0)
        {
            painter->setRenderHint(QPainter::Antialiasing, true);

            painter->setPen(QPen());
            painter->setBrush(QBrush(QColor(71, 143, 178)));

            constexpr int padding = 5;
            int circleRadius = (option.rect.height() / 2) - padding;

            QPoint circleCenter;
            circleCenter.setY(option.rect.top() + (option.rect.height() / 2));
            circleCenter.setX(option.rect.left() + (option.rect.height() / 2) + padding + (option.rect.height() * data.at(0).toInt()));

            painter->setClipRect(option.rect);
            painter->drawEllipse(circleCenter, circleRadius, circleRadius);
            painter->setClipping(false);
        }
    }
}
