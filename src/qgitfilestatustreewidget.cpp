// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitfilestatustreewidget.h"
#include <QApplication>
#include <QFileInfo>
#include <QHeaderView>
#include <QSet>
#include <QSignalBlocker>
#include <QStyle>

namespace {

constexpr int ItemKeyRole = Qt::UserRole + 10;

struct DesiredFileItem
{
    QString key;
    QString col0Text;
    QString col1Text;
    QString col2Text;
    QIcon icon;
    Qt::CheckState checkState = Qt::Unchecked;
    QString filePath;
    int status = 0;
    bool isFolder = false;
    QList<DesiredFileItem> children;
};

DesiredFileItem *findChildByKey(QList<DesiredFileItem> &list, const QString &key)
{
    for (auto &item : list) {
        if (item.key == key)
            return &item;
    }
    return nullptr;
}

void applyDesiredFileItem(QTreeWidgetItem *item, const DesiredFileItem &desired, int columnCount)
{
    if (item->data(0, ItemKeyRole).toString() != desired.key)
        item->setData(0, ItemKeyRole, desired.key);

    const Qt::ItemFlags flags = item->flags();
    if (!flags.testFlag(Qt::ItemIsUserCheckable))
        item->setFlags(flags | Qt::ItemIsUserCheckable);

    if (item->text(0) != desired.col0Text)
        item->setText(0, desired.col0Text);

    item->setIcon(0, desired.icon);

    if (!desired.isFolder && item->checkState(0) != desired.checkState)
        item->setCheckState(0, desired.checkState);

    if (desired.isFolder) {
        if (!item->data(0, Qt::UserRole).isNull())
            item->setData(0, Qt::UserRole, QVariant());
        if (!item->data(0, Qt::UserRole + 1).isNull())
            item->setData(0, Qt::UserRole + 1, QVariant());
    } else {
        if (item->data(0, Qt::UserRole).toString() != desired.filePath)
            item->setData(0, Qt::UserRole, desired.filePath);
        if (item->data(0, Qt::UserRole + 1).toInt() != desired.status)
            item->setData(0, Qt::UserRole + 1, desired.status);
    }

    if (columnCount >= 3) {
        if (item->text(1) != desired.col1Text)
            item->setText(1, desired.col1Text);
        if (item->text(2) != desired.col2Text)
            item->setText(2, desired.col2Text);
    }
}

void collectExpandedState(QTreeWidgetItem *item, QSet<QTreeWidgetItem *> &expanded)
{
    if (item->isExpanded())
        expanded.insert(item);
    for (int i = 0; i < item->childCount(); ++i)
        collectExpandedState(item->child(i), expanded);
}

void restoreExpandedState(QTreeWidgetItem *item, const QSet<QTreeWidgetItem *> &expanded)
{
    item->setExpanded(expanded.contains(item));
    for (int i = 0; i < item->childCount(); ++i)
        restoreExpandedState(item->child(i), expanded);
}

void syncFileTreeItems(QTreeWidgetItem *parentItem,
                       const QList<DesiredFileItem> &desiredChildren,
                       int columnCount)
{
    QSet<QString> desiredKeys;
    desiredKeys.reserve(desiredChildren.size());
    for (const auto &desired : desiredChildren)
        desiredKeys.insert(desired.key);

    for (int i = parentItem->childCount() - 1; i >= 0; --i) {
        QTreeWidgetItem *child = parentItem->child(i);
        if (!desiredKeys.contains(child->data(0, ItemKeyRole).toString()))
            delete parentItem->takeChild(i);
    }

    QHash<QString, QTreeWidgetItem *> existingByKey;
    existingByKey.reserve(parentItem->childCount());
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *child = parentItem->child(i);
        existingByKey.insert(child->data(0, ItemKeyRole).toString(), child);
    }

    for (int i = 0; i < desiredChildren.size(); ++i) {
        const DesiredFileItem &desired = desiredChildren.at(i);
        QTreeWidgetItem *child = existingByKey.value(desired.key, nullptr);

        if (child) {
            const int currentIndex = parentItem->indexOfChild(child);
            if (currentIndex != i) {
                QSet<QTreeWidgetItem *> expanded;
                collectExpandedState(child, expanded);
                QTreeWidget *tree = parentItem->treeWidget();
                QTreeWidgetItem *prevCurrent = tree ? tree->currentItem() : nullptr;

                parentItem->takeChild(currentIndex);
                parentItem->insertChild(i, child);

                restoreExpandedState(child, expanded);
                if (prevCurrent && tree)
                    tree->setCurrentItem(prevCurrent, 0, QItemSelectionModel::NoUpdate);
            }

            applyDesiredFileItem(child, desired, columnCount);
            syncFileTreeItems(child, desired.children, columnCount);
        } else {
            child = new QTreeWidgetItem();
            applyDesiredFileItem(child, desired, columnCount);
            parentItem->insertChild(i, child);
            syncFileTreeItems(child, desired.children, columnCount);
            if (desired.isFolder)
                child->setExpanded(true);
        }
    }
}

} // namespace

QGitFileStatusTreeWidget::QGitFileStatusTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(true);
    setColumnCount(1);
}

void QGitFileStatusTreeWidget::setViewLayoutMode(ViewLayoutMode mode)
{
    if (m_layoutMode != mode)
        clear();

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

QIcon QGitFileStatusTreeWidget::fileIcon(git_status_t status, bool isStaged)
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

QString QGitFileStatusTreeWidget::statusText(git_status_t status)
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
    const QList<std::pair<QString, git_status_t>> &files,
    FileCategory category)
{
    const QSignalBlocker blocker(this);
    setEnabled(true);

    QList<DesiredFileItem> desiredRoot;

    for (const auto &filePair : files)
    {
        const QString &file = filePair.first;
        git_status_t status = filePair.second;

        bool hasStaged = status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE);
        bool hasUnstaged = (status == GIT_STATUS_CURRENT) || (status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE | GIT_STATUS_IGNORED | GIT_STATUS_CONFLICTED));

        Qt::CheckState state = Qt::Unchecked;
        QIcon icon;

        if (category == FileCategory::Pending)
        {
            state = Qt::PartiallyChecked;
            if (hasStaged && !hasUnstaged) state = Qt::Checked;
            else if (!hasStaged && hasUnstaged) state = Qt::Unchecked;
            icon = fileIcon(status, hasStaged);
        }
        else if (category == FileCategory::Staged)
        {
            if (!hasStaged) continue;
            state = Qt::Checked;
            icon = fileIcon(status, true);
        }
        else
        {
            if (!hasUnstaged) continue;
            state = Qt::Unchecked;
            icon = fileIcon(status, false);
        }

        if (m_layoutMode == FlatSingle)
        {
            DesiredFileItem item;
            item.key = QStringLiteral("F:") + file;
            item.col0Text = file;
            item.icon = icon;
            item.checkState = state;
            item.filePath = file;
            item.status = static_cast<int>(status);
            desiredRoot.append(item);
        }
        else if (m_layoutMode == FlatMulti)
        {
            QFileInfo info(file);
            DesiredFileItem item;
            item.key = QStringLiteral("F:") + file;
            item.col0Text = info.fileName();
            item.col1Text = info.path() == QStringLiteral(".") ? QString() : info.path();
            item.col2Text = statusText(status);
            item.icon = icon;
            item.checkState = state;
            item.filePath = file;
            item.status = static_cast<int>(status);
            desiredRoot.append(item);
        }
        else if (m_layoutMode == TreeView)
        {
            QStringList parts = file.split('/');
            QList<DesiredFileItem> *currentList = &desiredRoot;
            for (int i = 0; i < parts.size() - 1; ++i)
            {
                const QString &dirName = parts.at(i);
                const QString dirKey = QStringLiteral("D:") + dirName;
                DesiredFileItem *found = findChildByKey(*currentList, dirKey);
                if (!found)
                {
                    DesiredFileItem folder;
                    folder.key = dirKey;
                    folder.col0Text = dirName;
                    folder.icon = QApplication::style()->standardIcon(QStyle::SP_DirIcon);
                    folder.isFolder = true;
                    currentList->append(folder);
                    currentList = &currentList->last().children;
                }
                else
                {
                    currentList = &found->children;
                }
            }

            DesiredFileItem item;
            item.key = QStringLiteral("F:") + parts.last();
            item.col0Text = parts.last();
            item.icon = icon;
            item.checkState = state;
            item.filePath = file;
            item.status = static_cast<int>(status);
            currentList->append(item);
        }
    }

    syncFileTreeItems(invisibleRootItem(), desiredRoot, columnCount());

    if (m_layoutMode == TreeView) {
        for (int i = 0; i < topLevelItemCount(); ++i) {
            updateFolderCheckStates(topLevelItem(i));
        }
    }
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
