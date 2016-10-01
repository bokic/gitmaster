#include "qgitrepotreewidget.h"
#include "qgitrepotreeitemdelegate.h"

QGitRepoTreeWidget::QGitRepoTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_thread(this)
    , m_git(new QGit())
    , m_refreshIndex(0)
{
    m_git->moveToThread(&m_thread);

    connect(this, SIGNAL(repositoryStatus()), m_git, SLOT(status()), Qt::QueuedConnection);
    connect(m_git, SIGNAL(statusReply(QMap<git_status_t,int>,QGitError)), this, SLOT(repositoryStatusReply(QMap<git_status_t,int>,QGitError)), Qt::QueuedConnection);

    m_thread.start();
}

QGitRepoTreeWidget::~QGitRepoTreeWidget()
{
    m_thread.quit();
    m_thread.wait();

    delete m_git; m_git = nullptr;
}

void QGitRepoTreeWidget::refreshItems()
{
    m_refreshIndex = 0;

    refreshItem();
}

void QGitRepoTreeWidget::repositoryStatusReply(QMap<git_status_t,int> items, QGitError error)
{
    Q_UNUSED(error);

    for(int index = 0; index < topLevelItemCount(); index++)
    {
        QTreeWidgetItem *item = topLevelItem(index);

        QString item_path = item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString();

        auto tmp = QDir::toNativeSeparators(m_git->path().absolutePath());

        if (item_path == QDir::toNativeSeparators(m_git->path().absolutePath()))
        {
            item->setData(0, QGitRepoTreeItemDelegate::QItemModifiedFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemDeletedFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemNewFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemUnversionedFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemBranchName, QGit::getBranchNameFromPath(item_path));

            for(int c = 0; c < items.count(); c++)
            {
                git_status_t key = items.keys()[c];

                if (key & (GIT_STATUS_WT_MODIFIED | GIT_STATUS_INDEX_MODIFIED))
                {
                    item->setData(0, QGitRepoTreeItemDelegate::QItemModifiedFiles, QVariant(item->data(0, QGitRepoTreeItemDelegate::QItemModifiedFiles).toInt() + items[key]));
                }

                if (key & (GIT_STATUS_WT_DELETED | GIT_STATUS_INDEX_DELETED))
                {
                    item->setData(0, QGitRepoTreeItemDelegate::QItemDeletedFiles, QVariant(item->data(0, QGitRepoTreeItemDelegate::QItemDeletedFiles).toInt() + items[key]));
                }

                if (key & (GIT_STATUS_WT_NEW | GIT_STATUS_INDEX_NEW))
                {
                    item->setData(0, QGitRepoTreeItemDelegate::QItemNewFiles, QVariant(item->data(0, QGitRepoTreeItemDelegate::QItemNewFiles).toInt() + items[key]));
                }
            }

            update();
            break;
        }
    }

    refreshItem();
}

void QGitRepoTreeWidget::refreshItem()
{
    if (m_refreshIndex >= topLevelItemCount())
    {
        m_refreshIndex = 0;

        return;
    }

    QTreeWidgetItem *item = topLevelItem(m_refreshIndex++);

    QDir dir = QDir(item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString());

    m_git->setPath(dir);

    emit repositoryStatus();
}
