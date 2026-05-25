#include "qloghistoryitemdelegate.h"
#include "qloghistorytablewidget.h"

#include <QPalette>
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>


QLogHistoryItemDelegate::QLogHistoryItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}

void QLogHistoryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        QStyledItemDelegate::paint(painter, option, index);

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
    else if (index.column() == 1)
    {
        auto *table = qobject_cast<const QLogHistoryTableWidget*>(option.widget);
        if (!table) {
            table = qobject_cast<const QLogHistoryTableWidget*>(parent());
        }

        QString sha = index.data(Qt::UserRole + 1).toString();
        QList<QGitRef> refs;
        if (table && !sha.isEmpty())
        {
            refs = table->getReferences(sha);
        }

        // Draw standard background, selection, focus, etc. but without text to avoid overlap
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text = "";
        
        auto *widget = option.widget;
        auto *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        int startX = option.rect.left() + 6;
        int rowH = option.rect.height();

        if (!refs.isEmpty())
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            // Font for badges: bold and slightly smaller
            QFont badgeFont = painter->font();
            badgeFont.setPointSizeF(badgeFont.pointSizeF() - 1.0);
            badgeFont.setBold(true);
            painter->setFont(badgeFont);

            QFontMetrics fm(badgeFont);

            for (const auto &ref : refs)
            {
                QString name = ref.name;
                int textW = fm.horizontalAdvance(name);
                int badgeH = fm.height() + 2; // small padding
                int badgeW = textW + 10; // horizontal padding

                // Ensure it fits within the option rect vertically
                int badgeY = option.rect.top() + (rowH - badgeH) / 2;

                QRect badgeRect(startX, badgeY, badgeW, badgeH);

                QColor bg, border, textCol;
                switch (ref.type)
                {
                case QGitRef::CurrentBranch:
                    bg = QColor(220, 245, 225);
                    border = QColor(30, 110, 45);
                    textCol = QColor(30, 110, 45);
                    break;
                case QGitRef::LocalBranch:
                    bg = QColor(225, 240, 255);
                    border = QColor(20, 80, 160);
                    textCol = QColor(20, 80, 160);
                    break;
                case QGitRef::RemoteBranch:
                    bg = QColor(255, 235, 230);
                    border = QColor(180, 50, 20);
                    textCol = QColor(180, 50, 20);
                    break;
                case QGitRef::Tag:
                    bg = QColor(255, 245, 200);
                    border = QColor(120, 80, 0);
                    textCol = QColor(120, 80, 0);
                    break;
                }

                // Draw badge background & border
                painter->setPen(QPen(border, 1));
                painter->setBrush(QBrush(bg));
                painter->drawRoundedRect(badgeRect, 3.0, 3.0);

                // Draw badge text
                painter->setPen(textCol);
                painter->drawText(badgeRect, Qt::AlignCenter, name);

                startX += badgeW + 4; // spacing between badges
            }

            painter->restore();
        }

        // Draw commit message text
        painter->save();
        QColor textColor;
        if (option.state & QStyle::State_Selected)
        {
            textColor = option.palette.color(QPalette::HighlightedText);
        }
        else
        {
            textColor = option.palette.color(QPalette::Text);
        }
        painter->setPen(textColor);

        QRect textRect = option.rect;
        textRect.setLeft(startX + 2);

        QString msg = index.data(Qt::DisplayRole).toString();
        QString elidedMsg = painter->fontMetrics().elidedText(msg, Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedMsg);
        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

