#include "qgitrepotreewidget.h"
#include "qgitrepotreeitemdelegate.h"


QGitRepoTreeWidget::QGitRepoTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_thread(this)
    , m_git(new QGit())
{
    m_git->moveToThread(&m_thread);

    connect(this, &QGitRepoTreeWidget::repositoryStatus, m_git, &QGit::status);
    connect(m_git, &QGit::statusReply, this, &QGitRepoTreeWidget::repositoryStatusReply);

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
    Q_UNUSED(error)

    for(int index = 0; index < topLevelItemCount(); index++)
    {
        QTreeWidgetItem *item = topLevelItem(index);

        QString item_path = item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString();

        if (item_path == QDir::toNativeSeparators(m_git->path().absolutePath()))
        {
            item->setData(0, QGitRepoTreeItemDelegate::QItemModifiedFiles, 0);
            item->setData(0, QGitRepoTreeItemDelegate::QItemDeletedFiles, 0);
            item->setData(0, QGitRepoTreeItemDelegate::QItemNewFiles, 0);
            item->setData(0, QGitRepoTreeItemDelegate::QItemUnversionedFiles, 0);
            item->setData(0, QGitRepoTreeItemDelegate::QItemBranchName, QGit::getBranchNameFromPath(item_path));

            for(const auto &[key, value]: items.asKeyValueRange())
            {
                if (key & (GIT_STATUS_WT_MODIFIED | GIT_STATUS_INDEX_MODIFIED))
                {
                    item->setData(0, QGitRepoTreeItemDelegate::QItemModifiedFiles, item->data(0, QGitRepoTreeItemDelegate::QItemModifiedFiles).toInt() + value);
                }

                if (key & (GIT_STATUS_WT_DELETED | GIT_STATUS_INDEX_DELETED))
                {
                    item->setData(0, QGitRepoTreeItemDelegate::QItemDeletedFiles, item->data(0, QGitRepoTreeItemDelegate::QItemDeletedFiles).toInt() + value);
                }

                if (key & (GIT_STATUS_WT_NEW | GIT_STATUS_INDEX_NEW))
                {
                    item->setData(0, QGitRepoTreeItemDelegate::QItemNewFiles, item->data(0, QGitRepoTreeItemDelegate::QItemNewFiles).toInt() + value);
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

    QDir dir(item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString());

    m_git->setPath(dir);

    emit repositoryStatus();
}
