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
        QFontMetrics fm(m_boldFont);

        painter->setFont(m_boldFont);
        painter->drawText(x, y + 17, text);

        x += fm.width(text) + 6;
    }

    text = index.data(Qt::UserRole + 1).toString();

    if (!text.isEmpty())
    {
        QFontMetrics fm(m_normalFont);

        painter->setFont(m_normalFont);
        painter->setPen(QPen(QPalette().color(QPalette::Disabled, QPalette::Text)));
        painter->drawText(x, y + 17, text);

        x += fm.width(text) + 6;
    }

    const QVariant &modifiedFiles = index.data(Qt::UserRole + 2);
    const QVariant &deletedFiles = index.data(Qt::UserRole + 3);
    const QVariant &addedFiles = index.data(Qt::UserRole + 4);
    const QVariant &unversionedFiles = index.data(Qt::UserRole + 5);
    const QVariant &branchName = index.data(Qt::UserRole + 6);

    if ((modifiedFiles.isValid())&&(deletedFiles.isValid())&&(addedFiles.isValid())&&(unversionedFiles.isValid())&&(branchName.isValid()))
    {

    }
}

QSize QGitRepoTreeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    // TODO: Calculate width.

    return QSize(200, 44);
}

void QGitRepoTreeItemDelegate::setImages(const QImage &branch_logo, const QImage &file_add, const QImage &file_modified, const QImage &file_removed, const QImage &file_unknown, const QImage &current_branch)
{
    m_branchLogoImage = branch_logo;
    m_fileStatusAddedImage = file_add;
    m_fileStatusModifiedImage = file_modified;
    m_fileStatusRemovedImage = file_removed;
    m_fileStatusUnknownImage = file_unknown;
    m_currentBranchImage = current_branch;
}
