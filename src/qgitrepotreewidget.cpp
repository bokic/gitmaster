#include "qgitrepotreewidget.h"
#include "qgitrepotreeitemdelegate.h"
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>


QGitRepoTreeWidget::QGitRepoTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_thread(this)
    , m_git(new QGit())
{
    setAcceptDrops(true);

    m_git->moveToThread(&m_thread);

    connect(this, &QGitRepoTreeWidget::repositoryStatus, m_git, &QGit::status);
    connect(m_git, &QGit::statusReply, this, &QGitRepoTreeWidget::repositoryStatusReply);

    m_thread.setObjectName("QGit(tree)");
    m_thread.start();
}

QGitRepoTreeWidget::~QGitRepoTreeWidget()
{
    m_git->disconnect(this);
    this->disconnect(m_git);

    m_thread.quit();
    m_thread.wait();

    delete m_git; m_git = nullptr;
}

void QGitRepoTreeWidget::refreshItems()
{
    if (m_isRefreshing)
    {
        m_refreshPending = true;
        return;
    }

    if (topLevelItemCount() == 0)
    {
        return;
    }

    m_isRefreshing = true;
    m_refreshPending = false;
    m_refreshIndex = 0;

    refreshItem();
}

void QGitRepoTreeWidget::repositoryStatusReply(QMap<git_status_t,int> items, QGitError error)
{
    if (!m_currentRefreshingPath.isEmpty())
    {
        QTreeWidgetItem *targetItem = nullptr;
        for (int index = 0; index < topLevelItemCount(); index++)
        {
            QTreeWidgetItem *item = topLevelItem(index);
            if (item && item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString() == m_currentRefreshingPath)
            {
                targetItem = item;
                break;
            }
        }

        if (targetItem && !error.hasError())
        {
            int modifiedCount = 0;
            int deletedCount = 0;
            int newCount = 0;
            int unversionedCount = 0;

            for (const auto &[key, value] : items.asKeyValueRange())
            {
                if (key & (GIT_STATUS_WT_MODIFIED | GIT_STATUS_INDEX_MODIFIED))
                {
                    modifiedCount += value;
                }
                if (key & (GIT_STATUS_WT_DELETED | GIT_STATUS_INDEX_DELETED))
                {
                    deletedCount += value;
                }
                if (key & (GIT_STATUS_WT_NEW | GIT_STATUS_INDEX_NEW))
                {
                    newCount += value;
                }
            }

            QString branchName = QGit::getBranchNameFromPath(m_currentRefreshingPath);

            bool changed = false;
            if (targetItem->data(0, QGitRepoTreeItemDelegate::QItemModifiedFiles) != modifiedCount) {
                targetItem->setData(0, QGitRepoTreeItemDelegate::QItemModifiedFiles, modifiedCount);
                changed = true;
            }
            if (targetItem->data(0, QGitRepoTreeItemDelegate::QItemDeletedFiles) != deletedCount) {
                targetItem->setData(0, QGitRepoTreeItemDelegate::QItemDeletedFiles, deletedCount);
                changed = true;
            }
            if (targetItem->data(0, QGitRepoTreeItemDelegate::QItemNewFiles) != newCount) {
                targetItem->setData(0, QGitRepoTreeItemDelegate::QItemNewFiles, newCount);
                changed = true;
            }
            if (targetItem->data(0, QGitRepoTreeItemDelegate::QItemUnversionedFiles) != unversionedCount) {
                targetItem->setData(0, QGitRepoTreeItemDelegate::QItemUnversionedFiles, unversionedCount);
                changed = true;
            }
            if (targetItem->data(0, QGitRepoTreeItemDelegate::QItemBranchName) != branchName) {
                targetItem->setData(0, QGitRepoTreeItemDelegate::QItemBranchName, branchName);
                changed = true;
            }

            if (changed) {
                update();
            }
        }
    }

    refreshItem();
}

void QGitRepoTreeWidget::refreshItem()
{
    if (m_refreshIndex >= topLevelItemCount())
    {
        m_isRefreshing = false;
        m_refreshIndex = 0;
        m_currentRefreshingPath.clear();

        if (m_refreshPending)
        {
            m_refreshPending = false;
            refreshItems();
        }
        return;
    }

    QTreeWidgetItem *item = topLevelItem(m_refreshIndex++);
    if (!item)
    {
        m_isRefreshing = false;
        m_refreshIndex = 0;
        m_currentRefreshingPath.clear();
        return;
    }

    m_currentRefreshingPath = item->data(0, QGitRepoTreeItemDelegate::QItemPath).toString();

    QDir dir(m_currentRefreshingPath);

    m_git->setPath(dir);

    emit repositoryStatus();
}

void QGitRepoTreeWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                QString localPath = url.toLocalFile();
                QFileInfo fi(localPath);
                if (fi.isDir()) {
                    QFileInfo gitInfo(QDir(localPath).filePath(".git"));
                    if (gitInfo.exists() || QGit::isGitRepository(QDir(localPath))) {
                        event->acceptProposedAction();
                        return;
                    }
                }
            }
        }
    }
    QTreeWidget::dragEnterEvent(event);
}

void QGitRepoTreeWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                QString localPath = url.toLocalFile();
                QFileInfo fi(localPath);
                if (fi.isDir()) {
                    QFileInfo gitInfo(QDir(localPath).filePath(".git"));
                    if (gitInfo.exists() || QGit::isGitRepository(QDir(localPath))) {
                        event->acceptProposedAction();
                        return;
                    }
                }
            }
        }
    }
    QTreeWidget::dragMoveEvent(event);
}

void QGitRepoTreeWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QStringList paths;
        for (const QUrl &url : event->mimeData()->urls()) {
            if (url.isLocalFile()) {
                QString localPath = url.toLocalFile();
                QFileInfo fi(localPath);
                if (fi.isDir()) {
                    QFileInfo gitInfo(QDir(localPath).filePath(".git"));
                    if (gitInfo.exists() || QGit::isGitRepository(QDir(localPath))) {
                        paths.append(localPath);
                    }
                }
            }
        }
        if (!paths.isEmpty()) {
            event->acceptProposedAction();
            emit repositoriesDropped(paths);
            return;
        }
    }
    QTreeWidget::dropEvent(event);
}

