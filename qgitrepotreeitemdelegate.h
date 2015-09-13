#ifndef QGITREPOTREEITEMDELEGATE_H
#define QGITREPOTREEITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QImage>
#include <QFont>

class QGitRepoTreeItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    enum QDelegateItems{QItemPath = Qt::UserRole + 1, QItemModifiedFiles, QItemDeletedFiles, QItemAddedFiles, QItemUnversionedFiles, QItemBranchName};
    explicit QGitRepoTreeItemDelegate(QObject *parent = 0);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setImages(const QImage &branch_logo, const QImage &file_ok, const QImage &file_add, const QImage &file_modified, const QImage &file_removed, const QImage &file_unknown, const QImage &current_branch);
signals:

public slots:

private:
    QFont m_boldFont;
    QFont m_normalFont;
    QImage m_branchLogoImage;
    QImage m_fileStatusOkImage;
    QImage m_fileStatusAddedImage;
    QImage m_fileStatusModifiedImage;
    QImage m_fileStatusRemovedImage;
    QImage m_fileStatusUnknownImage;
    QImage m_currentBranchImage;
};

#endif // QGITREPOTREEITEMDELEGATE_H
