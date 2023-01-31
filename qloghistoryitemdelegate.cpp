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
    QStyleOptionViewItem _option = option;

    if (index.column() == 0)
    {
        auto data = index.data(Qt::UserRole).toList();

        if (data.count() > 0)
        {
            QRectF circle = QRectF(_option.rect);
            int colorIndex = 0;

            circle.setLeft(0.5 + circle.left() + 4.0 + (circle.height() * data.at(0).toInt()));
            circle.setTop(0.5 + circle.top() + 4.0);
            circle.setHeight(circle.height() - 8.0);
            circle.setWidth(circle.height());

            painter->setClipRect(_option.rect);

            initStyleOption(&_option, index);

            // TODO: Something is weird here. Revisit when possible.
            if (_option.state & QStyle::State_Selected)
            {
                painter->fillRect(_option.rect, _option.palette.highlight());
            }
            else
            {
                painter->fillRect(_option.rect, _option.backgroundBrush);
            }

            painter->setRenderHint(QPainter::Antialiasing, true);

            painter->setPen(QPen());
            painter->setBrush(QBrush(m_colors[colorIndex % m_colors.count()]));
            painter->drawEllipse(circle);
        }
    }
    else
    {
        QStyledItemDelegate::paint(painter, _option, index);
    }
}
