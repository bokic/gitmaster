#include "qgitfilestatustreewidget.h"
#include <QApplication>
#include <QStyle>
#include <QFileInfo>
#include <QHeaderView>

QGitFileStatusTreeWidget::QGitFileStatusTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(true);
    setColumnCount(1);
}

void QGitFileStatusTreeWidget::setViewLayoutMode(ViewLayoutMode mode)
{
    m_layoutMode = mode;

    if (m_layoutMode == FlatMulti)
    {
        setHeaderHidden(false);
        setColumnCount(3);
        QStringList headers = { tr("Name"), tr("Path"), tr("Status") };
        setHeaderLabels(headers);

        header()->setSectionResizeMode(0, QHeaderView::Interactive);
        header()->setSectionResizeMode(1, QHeaderView::Stretch);
        header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }
    else
    {
        setHeaderHidden(true);
        setColumnCount(1);
    }
}

QIcon QGitFileStatusTreeWidget::getFileIcon(git_status_t status, bool isStaged)
{
    static QIcon iconNew(":/small/added");
    static QIcon iconClean(":/small/clean");
    static QIcon iconModified(":/small/modified");
    static QIcon iconRemoved(":/small/deleted");
    static QIcon iconRenamed(":/small/images/small/renamed.png");
    static QIcon iconConflict(":/small/images/small/conflict.png");
    static QIcon iconIgnored(":/small/ignored");
    static QIcon iconUnknown(":/small/unknown");

    if (isStaged) {
        if (status & GIT_STATUS_INDEX_NEW) return iconNew;
        else if (status & GIT_STATUS_INDEX_DELETED) return iconRemoved;
        else if (status & GIT_STATUS_INDEX_RENAMED) return iconRenamed;
        else return iconModified;
    } else {
        uint32_t wt_status = status & (GIT_STATUS_CURRENT | GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED | GIT_STATUS_WT_RENAMED);
        if (wt_status == GIT_STATUS_CURRENT) return iconClean;
        else if (wt_status & GIT_STATUS_WT_NEW) return iconNew;
        else if (wt_status & GIT_STATUS_WT_MODIFIED) return iconModified;
        else if (wt_status & GIT_STATUS_WT_DELETED) return iconRemoved;
        else if (wt_status & GIT_STATUS_WT_RENAMED) return iconRenamed;
        else if (wt_status & GIT_STATUS_IGNORED) return iconIgnored;
        else if (wt_status & GIT_STATUS_CONFLICTED) return iconConflict;
        else return iconUnknown;
    }
}

QString QGitFileStatusTreeWidget::getStatusText(git_status_t status)
{
    if (status & GIT_STATUS_CONFLICTED) return tr("Conflicted");
    if (status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_WT_NEW)) return tr("Added");
    if (status & (GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_WT_MODIFIED)) return tr("Modified");
    if (status & (GIT_STATUS_INDEX_DELETED | GIT_STATUS_WT_DELETED)) return tr("Deleted");
    if (status & (GIT_STATUS_INDEX_RENAMED | GIT_STATUS_WT_RENAMED)) return tr("Renamed");
    if (status & (GIT_STATUS_INDEX_TYPECHANGE | GIT_STATUS_WT_TYPECHANGE)) return tr("Typechange");
    if (status & GIT_STATUS_IGNORED) return tr("Ignored");
    return tr("Unknown");
}

void QGitFileStatusTreeWidget::populateFiles(
    const QList<QPair<QString, git_status_t>> &files,
    bool isStagedCategory,
    bool isPendingCategory)
{
    blockSignals(true);
    clear();
    setEnabled(true);

    for (const auto &filePair : files)
    {
        const QString &file = filePair.first;
        git_status_t status = filePair.second;

        bool hasStaged = status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE);
        bool hasUnstaged = (status == GIT_STATUS_CURRENT) || (status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED));

        Qt::CheckState state = Qt::Unchecked;
        QIcon icon;

        if (isPendingCategory)
        {
            state = Qt::PartiallyChecked;
            if (hasStaged && !hasUnstaged) state = Qt::Checked;
            else if (!hasStaged && hasUnstaged) state = Qt::Unchecked;
            icon = getFileIcon(status, hasStaged);
        }
        else if (isStagedCategory)
        {
            if (!hasStaged) continue;
            state = Qt::Checked;
            icon = getFileIcon(status, true);
        }
        else
        {
            if (!hasUnstaged) continue;
            state = Qt::Unchecked;
            icon = getFileIcon(status, false);
        }

        if (m_layoutMode == FlatSingle)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(this);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, state);
            item->setData(0, Qt::UserRole, file);
            item->setData(0, Qt::UserRole + 1, (int)status);
            item->setText(0, file);
            item->setIcon(0, icon);
        }
        else if (m_layoutMode == FlatMulti)
        {
            QTreeWidgetItem *item = new QTreeWidgetItem(this);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, state);
            item->setData(0, Qt::UserRole, file);
            item->setData(0, Qt::UserRole + 1, (int)status);

            QFileInfo info(file);
            item->setText(0, info.fileName());
            item->setIcon(0, icon);
            item->setText(1, info.path() == "." ? "" : info.path());
            item->setText(2, getStatusText(status));
        }
        else if (m_layoutMode == TreeView)
        {
            QStringList parts = file.split('/');
            QTreeWidgetItem *parent = nullptr;
            for (int i = 0; i < parts.size() - 1; ++i)
            {
                QString dirName = parts.at(i);
                QTreeWidgetItem *found = nullptr;
                int childCount = parent ? parent->childCount() : topLevelItemCount();
                for (int j = 0; j < childCount; ++j)
                {
                    QTreeWidgetItem *child = parent ? parent->child(j) : topLevelItem(j);
                    if (child->text(0) == dirName && child->data(0, Qt::UserRole).toString().isEmpty())
                    {
                        found = child;
                        break;
                    }
                }
                if (!found)
                {
                    parent = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
                    parent->setText(0, dirName);
                    parent->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
                    parent->setFlags(parent->flags() | Qt::ItemIsUserCheckable);
                    parent->setCheckState(0, Qt::Unchecked);
                }
                else
                {
                    parent = found;
                }
            }

            QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem(this);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(0, state);
            item->setData(0, Qt::UserRole, file);
            item->setData(0, Qt::UserRole + 1, (int)status);
            item->setText(0, parts.last());
            item->setIcon(0, icon);
        }
    }

    if (m_layoutMode == TreeView) {
        for (int i = 0; i < topLevelItemCount(); ++i) {
            updateFolderCheckStates(topLevelItem(i));
        }
        expandAll();
    }

    blockSignals(false);
}

void QGitFileStatusTreeWidget::updateFolderCheckStates(QTreeWidgetItem *item)
{
    if (!item) return;

    int checkedCount = 0;
    int partiallyCheckedCount = 0;
    int totalChildren = item->childCount();

    if (totalChildren == 0) return;

    for (int i = 0; i < totalChildren; ++i)
    {
        QTreeWidgetItem *child = item->child(i);
        updateFolderCheckStates(child);

        Qt::CheckState state = child->checkState(0);
        if (state == Qt::Checked) {
            checkedCount++;
        } else if (state == Qt::PartiallyChecked) {
            partiallyCheckedCount++;
        }
    }

    if (checkedCount == totalChildren) {
        item->setCheckState(0, Qt::Checked);
    } else if (checkedCount > 0 || partiallyCheckedCount > 0) {
        item->setCheckState(0, Qt::PartiallyChecked);
    } else {
        item->setCheckState(0, Qt::Unchecked);
    }
}
