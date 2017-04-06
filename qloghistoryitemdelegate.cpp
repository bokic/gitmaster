#include "qloghistoryitemdelegate.h"
#include <QPainter>


QLogHistoryItemDelegate::QLogHistoryItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{

}

void QLogHistoryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        /*if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
        }*/

        QList<QVariant> data = index.data(Qt::UserRole).toList();

        if (data.count() > 0)
        {
            auto circle = QRectF(option.rect);

            circle.setLeft(circle.left() + 5.0 + (circle.height() * data.at(0).toInt()));
            circle.setTop(circle.top() + 5.0);
            circle.setHeight(circle.height() - 8.0);
            circle.setWidth(circle.height());

            auto pen = QPen(Qt::black);
            pen.setWidthF(1.5);

            painter->setRenderHint(QPainter::Antialiasing, true);

            painter->setBrush(QBrush(Qt::red, Qt::SolidPattern));
            painter->setPen(pen);
            painter->drawEllipse(circle);
        }
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}
