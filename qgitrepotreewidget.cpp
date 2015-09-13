#include "qgitrepotreewidget.h"
#include "qgitrepotreeitemdelegate.h"

QGitRepoTreeWidget::QGitRepoTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_thread(this)
    , m_git(new QGit())
{
    m_git->moveToThread(&m_thread);

    connect(this, SIGNAL(repositoryStatus(QDir)), m_git, SLOT(repositoryStatus(QDir)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(repositoryStatusReply(QDir,QHash<git_status_t,int>)), this, SLOT(repositoryStatusReply(QDir,QHash<git_status_t,int>)), Qt::QueuedConnection);

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
    for(int index = 0; index < topLevelItemCount(); index++)
    {
        QTreeWidgetItem *item = topLevelItem(index);

        emit repositoryStatus(QDir(item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString()));
    }
}

void QGitRepoTreeWidget::repositoryStatusReply(QDir path, QHash<git_status_t, int> items)
{
    for(int index = 0; index < topLevelItemCount(); index++)
    {
        QTreeWidgetItem *item = topLevelItem(index);

        QString item_path = item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString();

        if (item_path == QDir::toNativeSeparators(path.absolutePath()))
        {
            item->setData(0, QGitRepoTreeItemDelegate::QItemModifiedFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemDeletedFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemAddedFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemUnversionedFiles, QVariant(0));
            item->setData(0, QGitRepoTreeItemDelegate::QItemBranchName, QGit::getBranchNameFromPath(item_path));

            for(int c = 0; c < items.count(); c++)
            {
                git_status_t key = items.keys()[c];

                switch(key)
                {
                case GIT_STATUS_WT_MODIFIED:
                    item->setData(0, QGitRepoTreeItemDelegate::QItemModifiedFiles, QVariant(items[key]));
                    break;
                case GIT_STATUS_WT_DELETED:
                    item->setData(0, QGitRepoTreeItemDelegate::QItemDeletedFiles, QVariant(items[key]));
                    break;
                case GIT_STATUS_WT_NEW:
                    item->setData(0, QGitRepoTreeItemDelegate::QItemAddedFiles, QVariant(items[key]));
                    break;
                default:
                    break;
                }
            }

            update();
            break;
        }
    }
}
