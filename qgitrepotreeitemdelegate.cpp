#include "qgitrepotreeitemdelegate.h"

#include <QStandardPaths>
#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QColor>
#include <QStyle>


QGitRepoTreeItemDelegate::QGitRepoTreeItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_boldFont("Arial", 10, QFont::Bold, false)
    , m_normalFont("Arial", 10, QFont::Normal, false)
{
}

void QGitRepoTreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fm(m_normalFont);
    QFontMetrics fmb(m_boldFont);
    QColor background, text_color;
    QString text;

    if (option.state & QStyle::State_Selected) {
        background = option.palette.highlight().color();
        text_color = option.palette.highlightedText().color();
    } else {
        background = option.palette.base().color();
        text_color = option.palette.text().color();
    }

    auto modifiedFiles = index.data(QItemModifiedFiles);
    auto deletedFiles = index.data(QItemDeletedFiles);
    auto newFiles = index.data(QItemNewFiles);
    auto unversionedFiles = index.data(QItemUnversionedFiles);
    auto branchName = index.data(QItemBranchName);

    int x = option.rect.left();
    int y = option.rect.top();

    painter->fillRect(option.rect, background);

    if(!m_branchLogoImage.isNull())
    {
        painter->drawImage(x, y + 5, m_branchLogoImage);

        x += m_branchLogoImage.width() + 2;
    }

    text = index.data(Qt::DisplayRole).toString();

    painter->setPen(text_color);

    if (!text.isEmpty())
    {
        painter->setFont(m_boldFont);
        painter->drawText(x, y + 17, text);

        x += fmb.horizontalAdvance(text) + 6;
    }

    text = index.data(QItemPath).toString();

    if (!text.isEmpty())
    {

#ifdef Q_OS_UNIX
        auto homePath = QStandardPaths::locate(QStandardPaths::HomeLocation, "", QStandardPaths::LocateDirectory);
        if (text.startsWith(homePath))
        {
            text = QString("~/") + text.right(text.length() - homePath.length());
        }
#endif
        painter->setFont(m_normalFont);
        painter->drawText(x, y + 17, text);

        //x += fm.width(text) + 6;
    }

    x = option.rect.x() + 10;
    y = option.rect.y() + 24;


    if ((modifiedFiles.isValid())&&(deletedFiles.isValid())&&(newFiles.isValid())&&(unversionedFiles.isValid())&&(branchName.isValid()))
    {

        painter->setFont(m_normalFont);

        if (modifiedFiles.toInt() > 0)
        {
            painter->drawImage(x, y, m_fileStatusModifiedImage);

            x += m_fileStatusModifiedImage.width() + 1;

            text = QString::number(modifiedFiles.toInt());
            painter->drawText(x, y + fm.height() - fm.descent(), text);
            x += fm.horizontalAdvance(text) + 4;
        }

        if (deletedFiles.toInt() > 0)
        {
            painter->drawImage(x, y, m_fileStatusRemovedImage);

            x += m_fileStatusRemovedImage.width() + 1;

            text = QString::number(deletedFiles.toInt());
            painter->drawText(x, y + fm.height() - fm.descent(), text);
            x += fm.horizontalAdvance(text) + 4;
        }

        if (newFiles.toInt() > 0)
        {
            painter->drawImage(x, y, m_fileStatusNewImage);

            x += m_fileStatusNewImage.width() + 1;

            text = QString::number(newFiles.toInt());
            painter->drawText(x, y + fm.height() - fm.descent(), text);
            x += fm.horizontalAdvance(text) + 4;
        }

        if ((modifiedFiles.toInt() == 0)&&(deletedFiles.toInt() == 0)&&(newFiles.toInt() == 0)) {
            painter->drawImage(x, y, m_fileStatusOkImage);
            x += m_fileStatusOkImage.width() + 4;
        }
    }

    text = branchName.toString();

    if (!text.isEmpty()) {
        // draw vertical line
        painter->drawLine(x, y + 1, x, y + 15);
        x += 3;

        // draw branch name
        painter->drawText(x, y + fm.height() - fm.descent(), text);
    }
}

QSize QGitRepoTreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //int y = 0;

    Q_UNUSED(option)
    Q_UNUSED(index)

    //QFontMetrics f1(m_normalFont);
    //QFontMetrics f2(m_boldFont);

    //y = (option.fontMetrics.height() * 2) + 4;

    return {200, 44};
}

void QGitRepoTreeItemDelegate::setImages(const QImage &branch_logo, const QImage &file_ok, const QImage &file_new, const QImage &file_modified, const QImage &file_removed, const QImage &current_branch)
{
    m_branchLogoImage = branch_logo;
    m_fileStatusOkImage = file_ok;
    m_fileStatusNewImage = file_new;
    m_fileStatusModifiedImage = file_modified;
    m_fileStatusRemovedImage = file_removed;
    m_currentBranchImage = current_branch;
}
