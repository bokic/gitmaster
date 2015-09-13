#include "qgitrepotreeitemdelegate.h"
#include <QPainter>
#include <QColor>

QGitRepoTreeItemDelegate::QGitRepoTreeItemDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
    , m_boldFont("Arial", 10, QFont::Bold, false)
    , m_normalFont("Arial", 10, QFont::Normal, false)
{
}

void QGitRepoTreeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFontMetrics fm(m_normalFont);
    QFontMetrics fmb(m_boldFont);
    QString text;

    painter->setPen(QPen(Qt::NoPen));
    if (option.state & QStyle::State_Selected) {
        if (option.state & QStyle::State_Active) {
            painter->setBrush(QBrush(QPalette().color(QPalette::Active, QPalette::Highlight)));
        } else {
            painter->setBrush(QBrush(QPalette().color(QPalette::Inactive, QPalette::Highlight)));
        }
    } else {
        if (option.state & QStyle::State_Active) {
            painter->setBrush(QBrush(QPalette().color(QPalette::Active, QPalette::Base)));
        } else {
            painter->setBrush(QBrush(QPalette().color(QPalette::Inactive, QPalette::Base)));
        }
    }

    painter->drawRect(option.rect);

    painter->setPen(QPen(QPalette().color(QPalette::Active, QPalette::Text)));

    int x = option.rect.left();
    int y = option.rect.top();

    if(!m_branchLogoImage.isNull())
    {
        painter->drawImage(x, y + 5, m_branchLogoImage);

        x += m_branchLogoImage.width() + 2;
    }

    text = index.data(Qt::DisplayRole).toString();

    if (!text.isEmpty())
    {
        painter->setFont(m_boldFont);
        painter->drawText(x, y + 17, text);

        x += fmb.width(text) + 6;
    }

    text = index.data(QItemPath).toString();

    if (!text.isEmpty())
    {
        painter->setFont(m_normalFont);
        painter->setPen(QPen(QPalette().color(QPalette::Disabled, QPalette::Text)));
        painter->drawText(x, y + 17, text);

        x += fm.width(text) + 6;
    }

    const QVariant &modifiedFiles = index.data(QItemModifiedFiles);
    const QVariant &deletedFiles = index.data(QItemDeletedFiles);
    const QVariant &newFiles = index.data(QItemNewFiles);
    const QVariant &unversionedFiles = index.data(QItemUnversionedFiles);
    const QVariant &branchName = index.data(QItemBranchName);

    x = option.rect.x() + 10;
    y = option.rect.y() + 24;


    if ((modifiedFiles.isValid())&&(deletedFiles.isValid())&&(newFiles.isValid())&&(unversionedFiles.isValid())&&(branchName.isValid()))
    {

        painter->setFont(m_normalFont);
        painter->setPen(QPen(QPalette().color(QPalette::Active, QPalette::Text)));

        if (modifiedFiles.toInt() > 0)
        {
            painter->drawImage(x, y, m_fileStatusModifiedImage);

            x += m_fileStatusModifiedImage.width() + 1;

            text = QString::number(modifiedFiles.toInt());

            painter->drawText(x, y + fm.height() - fm.descent(), text);

            x += fm.width(text) + 4;
        }

        if (deletedFiles.toInt() > 0)
        {
            painter->drawImage(x, y, m_fileStatusRemovedImage);

            x += m_fileStatusRemovedImage.width() + 1;

            text = QString::number(deletedFiles.toInt());

            painter->drawText(x, y + fm.height() - fm.descent(), text);

            x += fm.width(text) + 4;
        }

        if (newFiles.toInt() > 0)
        {
            painter->drawImage(x, y, m_fileStatusNewImage);

            x += m_fileStatusNewImage.width() + 1;

            text = QString::number(newFiles.toInt());

            painter->drawText(x, y + fm.height() - fm.descent(), text);

            x += fm.width(text) + 4;
        }

        if ((modifiedFiles.toInt() == 0)&&(deletedFiles.toInt() == 0)&&(newFiles.toInt() == 0)) {
            painter->drawImage(x, y, m_fileStatusOkImage);

            x += m_fileStatusOkImage.width() + 4;
        }
    }

    // draw vertical line
    painter->drawLine(x, y + 1, x, y + 15);
    x += 3;

    // draw branch name
    painter->drawText(x, y + fm.height() - fm.descent(), branchName.toString());
}

QSize QGitRepoTreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // TODO: Calculate width.

    return QSize(200, 44);
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
