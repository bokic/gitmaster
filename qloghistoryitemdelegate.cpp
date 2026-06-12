#include "qloghistoryitemdelegate.h"
#include "qloghistorytablewidget.h"

#include <QPalette>
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>


#include <QPainterPath>


QLogHistoryItemDelegate::QLogHistoryItemDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}



void QLogHistoryItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 0)
    {
        // First draw standard cell background/selection
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text = "";
        auto *widget = option.widget;
        auto *style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

        auto data = index.data(Qt::UserRole).toList();

        auto *table = qobject_cast<const QLogHistoryTableWidget*>(option.widget);
        if (!table) {
            table = qobject_cast<const QLogHistoryTableWidget*>(parent());
        }

        QString sha;
        if (table)
        {
            auto sibling = index.sibling(index.row(), 1);
            sha = sibling.data(Qt::UserRole + 1).toString();
        }

        bool isCurrentCommit = false;
        if (table && !sha.isEmpty())
        {
            auto refs = table->getReferences(sha);
            for (const auto &ref : refs)
            {
                if (ref.type == QGitRef::CurrentBranch)
                {
                    isCurrentCommit = true;
                    break;
                }
            }
        }

        if (data.count() >= 4)
        {
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, true);

            int commitLane = data.at(0).toInt();
            auto passingLines = data.at(1).toList();
            auto parentConnections = data.at(2).toList();
            auto activeLanesAtTop = data.at(3).toList();



            int rowH = option.rect.height();
            constexpr int padding = 5;
            int circleRadius = (rowH / 2) - padding;
            if (circleRadius < 4) circleRadius = 4;

            auto getLaneColor = [](int lane) -> QColor {
                static const QList<QColor> colors = {
                    QColor(71, 143, 178),   // Blue
                    QColor(112, 191, 112),  // Green
                    QColor(230, 126, 34),   // Orange
                    QColor(155, 89, 182),   // Purple
                    QColor(231, 76, 60),    // Red
                    QColor(241, 196, 15),   // Yellow
                    QColor(52, 152, 219),   // Light Blue
                    QColor(26, 188, 156)    // Turquoise
                };
                return colors.at(lane % colors.size());
            };

            auto getLaneX = [&](int lane) -> int {
                return option.rect.left() + (rowH / 2) + padding + (rowH * lane);
            };

            int topY = option.rect.top();
            int centerY = option.rect.top() + (rowH / 2);
            int bottomY = option.rect.top() + rowH;

            // 1. Draw passing lines (straight vertical)
            for (const auto &laneVar : passingLines)
            {
                int lane = laneVar.toInt();
                int x = getLaneX(lane);
                painter->setPen(QPen(getLaneColor(lane), 2, Qt::SolidLine, Qt::RoundCap));
                painter->drawLine(QPoint(x, topY), QPoint(x, bottomY));
            }

            int commitX = getLaneX(commitLane);

            // 2. Draw line from top to node for commitLane (if active at the top)
            bool activeAtTop = false;
            for (const auto &laneVar : activeLanesAtTop)
            {
                if (laneVar.toInt() == commitLane)
                {
                    activeAtTop = true;
                    break;
                }
            }
            if (activeAtTop)
            {
                painter->setPen(QPen(getLaneColor(commitLane), 2, Qt::SolidLine, Qt::RoundCap));
                painter->drawLine(QPoint(commitX, topY), QPoint(commitX, centerY));
            }

            // 3. Draw parent connections
            for (int i = 0; i < parentConnections.size() - 1; i += 2)
            {
                int from = parentConnections.at(i).toInt();
                int to = parentConnections.at(i + 1).toInt();
                int fromX = getLaneX(from);
                int toX = getLaneX(to);

                if (from == to)
                {
                    // Straight vertical line to bottom
                    painter->setPen(QPen(getLaneColor(from), 2, Qt::SolidLine, Qt::RoundCap));
                    painter->drawLine(QPoint(fromX, centerY), QPoint(fromX, bottomY));
                }
                else
                {
                    // Straight diagonal split/merge line to bottom
                    painter->setPen(QPen(getLaneColor(to), 2, Qt::SolidLine, Qt::RoundCap));
                    painter->drawLine(QPoint(fromX, centerY), QPoint(toX, bottomY));
                }
            }

            // 4. Draw child connections (merging from top to center)
            if (data.count() >= 5)
            {
                auto childConnections = data.at(4).toList();
                for (int i = 0; i < childConnections.size() - 1; i += 2)
                {
                    int from = childConnections.at(i).toInt();
                    int to = childConnections.at(i + 1).toInt();
                    int fromX = getLaneX(from);
                    int toX = getLaneX(to);

                    // Straight child diagonal merge line from top
                    painter->setPen(QPen(getLaneColor(from), 2, Qt::SolidLine, Qt::RoundCap));
                    painter->drawLine(QPoint(fromX, topY), QPoint(toX, centerY));
                }
            }

            // 5. Draw the commit node circle
            QPoint circleCenter(commitX, centerY);
            painter->setPen(QPen(Qt::black, 1));
            painter->setBrush(QBrush(getLaneColor(commitLane)));
            painter->drawEllipse(circleCenter, circleRadius, circleRadius);

            // Premium visual design highlight: inner white circle for current checked-out commit
            if (isCurrentCommit)
            {
                painter->setBrush(QBrush(Qt::white));
                painter->drawEllipse(circleCenter, circleRadius / 2, circleRadius / 2);
            }

            painter->restore();
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

        // Check if this commit represents the current checked-out HEAD
        bool isCurrentCommit = false;
        for (const auto &ref : refs)
        {
            if (ref.type == QGitRef::CurrentBranch)
            {
                isCurrentCommit = true;
                break;
            }
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

        if (isCurrentCommit)
        {
            QFont boldF = painter->font();
            boldF.setBold(true);
            painter->setFont(boldF);
        }

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

