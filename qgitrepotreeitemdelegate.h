#pragma once

#include <QStyledItemDelegate>
#include <QImage>
#include <QFont>

class QGitRepoTreeItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    enum QDelegateItems{QItemPath = Qt::UserRole + 1, QItemModifiedFiles, QItemDeletedFiles, QItemNewFiles, QItemUnversionedFiles, QItemBranchName};
    explicit QGitRepoTreeItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setImages(const QImage &branch_logo, const QImage &file_ok, const QImage &file_new, const QImage &file_modified, const QImage &file_removed, const QImage &current_branch);
signals:

public slots:

private:
    QFont m_boldFont;
    QFont m_normalFont;
    QImage m_branchLogoImage;
    QImage m_fileStatusOkImage;
    QImage m_fileStatusNewImage;
    QImage m_fileStatusModifiedImage;
    QImage m_fileStatusRemovedImage;
    QImage m_currentBranchImage;
};
