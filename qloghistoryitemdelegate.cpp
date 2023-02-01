#include "qloghistoryitemdelegate.h"

#include <QPalette>
#include <QPainter>


QLogHistoryItemDelegate::QLogHistoryItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
    , m_colors({QColor(71, 143, 178)})
{
}

void QLogHistoryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        auto data = index.data(Qt::UserRole).toList();

        if (data.count() > 0)
        {
            int colorIndex = 0;

            if (option.state & QStyle::State_Selected)
            {
                painter->fillRect(option.rect, option.palette.highlight());
            }
            else
            {
                painter->fillRect(option.rect, option.backgroundBrush);
            }

            painter->setRenderHint(QPainter::Antialiasing, true);

            painter->setPen(QPen());
            painter->setBrush(QBrush(m_colors[colorIndex % m_colors.count()]));

            QPoint circleCenter;
            int circleRadius = 0;
            constexpr int padding = 4;
            circleRadius = (option.rect.height() / 2) - padding;
            circleCenter.setY(option.rect.top() + (option.rect.height() / 2));
            circleCenter.setX(option.rect.left() + (option.rect.height() / 2) + padding + (option.rect.height() * data.at(0).toInt()));
            //painter->setClipRect(option.rect);
            painter->drawEllipse(circleCenter, circleRadius, circleRadius);
            //painter->setClipping(false);
        }
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
