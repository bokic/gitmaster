// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitbranchtreewidget.h"
#include <QMap>
#include <QPainter>
#include <QPixmap>
#include <QSet>
#include <QSignalBlocker>
#include <algorithm>

namespace {

constexpr int ItemKeyRole = Qt::UserRole + 10;

struct DesiredItem
{
    QString key;
    QString text;
    QIcon icon;
    QString toolTip;
    bool isBold = false;
    bool isEditable = false;
    bool expandIfNew = true;
    QMap<int, QVariant> data;
    QList<DesiredItem> children;
};

void applyDesiredItem(QTreeWidgetItem *item, const DesiredItem &desired)
{
    if (item->data(0, ItemKeyRole).toString() != desired.key)
        item->setData(0, ItemKeyRole, desired.key);

    if (item->text(0) != desired.text)
        item->setText(0, desired.text);

    item->setIcon(0, desired.icon);

    if (item->toolTip(0) != desired.toolTip)
        item->setToolTip(0, desired.toolTip);

    QFont font = item->font(0);
    if (font.bold() != desired.isBold) {
        font.setBold(desired.isBold);
        item->setFont(0, font);
    }

    const Qt::ItemFlags flags = item->flags();
    if (flags.testFlag(Qt::ItemIsEditable) != desired.isEditable) {
        item->setFlags(desired.isEditable ? (flags | Qt::ItemIsEditable)
                                          : (flags & ~Qt::ItemIsEditable));
    }

    for (int role = Qt::UserRole; role <= Qt::UserRole + 4; ++role) {
        const QVariant val = desired.data.value(role);
        if (item->data(0, role) != val)
            item->setData(0, role, val);
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

void syncTreeItems(QTreeWidgetItem *parentItem,
                   const QList<DesiredItem> &desiredChildren,
                   const QSet<QString> &preserveKeys = {})
{
    QSet<QString> desiredKeys;
    desiredKeys.reserve(desiredChildren.size());
    for (const auto &desired : desiredChildren)
        desiredKeys.insert(desired.key);

    for (int i = parentItem->childCount() - 1; i >= 0; --i) {
        QTreeWidgetItem *child = parentItem->child(i);
        const QString key = child->data(0, ItemKeyRole).toString();
        if (!desiredKeys.contains(key) && !preserveKeys.contains(key))
            delete parentItem->takeChild(i);
    }

    QHash<QString, QTreeWidgetItem *> existingByKey;
    existingByKey.reserve(parentItem->childCount());
    for (int i = 0; i < parentItem->childCount(); ++i) {
        QTreeWidgetItem *child = parentItem->child(i);
        existingByKey.insert(child->data(0, ItemKeyRole).toString(), child);
    }

    for (int i = 0; i < desiredChildren.size(); ++i) {
        const DesiredItem &desired = desiredChildren.at(i);
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

            applyDesiredItem(child, desired);
            syncTreeItems(child, desired.children);
        } else {
            child = new QTreeWidgetItem();
            applyDesiredItem(child, desired);
            parentItem->insertChild(i, child);
            syncTreeItems(child, desired.children);
            if (desired.expandIfNew)
                child->setExpanded(true);
        }
    }
}

DesiredItem *findChildByKey(QList<DesiredItem> &list, const QString &key)
{
    for (auto &item : list) {
        if (item.key == key)
            return &item;
    }
    return nullptr;
}

QIcon makeSubmoduleBadgeIcon(const QIcon &base, const QColor &badgeColor)
{
    constexpr int iconSize = 16;
    QPixmap pix = base.pixmap(iconSize, iconSize);
    if (pix.isNull())
        return base;

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::white, 1.0));
    painter.setBrush(badgeColor);
    painter.drawEllipse(QRectF(9.5, 9.5, 6.0, 6.0));
    return QIcon(pix);
}

QIcon makeSubmoduleDimmedIcon(const QIcon &base)
{
    constexpr int iconSize = 16;
    QPixmap src = base.pixmap(iconSize, iconSize);
    if (src.isNull())
        return base;

    QPixmap dst(src.size());
    dst.setDevicePixelRatio(src.devicePixelRatio());
    dst.fill(Qt::transparent);

    QPainter painter(&dst);
    painter.setOpacity(0.45);
    painter.drawPixmap(0, 0, src);
    return QIcon(dst);
}

} // namespace

QGitBranchTreeWidget::QGitBranchTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_iconTag(":/small/tag")
    , m_iconWhiteCheckbox(":/small/white_checkbox")
    , m_iconCurrentBranch(":/small/current_branch")
    , m_iconBranch(":/small/branch")
    , m_iconRemote(":/small/remote")
    , m_iconStash(":/small/stash")
    , m_iconRemoteBranch(":/small/remote_branch")
    , m_iconSubmodule(":/small/submodule")
    , m_iconSubmoduleCommitChanged(makeSubmoduleBadgeIcon(m_iconSubmodule, QColor(0x21, 0x96, 0xF3)))
    , m_iconSubmoduleDirty(makeSubmoduleBadgeIcon(m_iconSubmodule, QColor(0xF5, 0x7C, 0x00)))
    , m_iconSubmoduleUninitialized(makeSubmoduleDimmedIcon(m_iconSubmodule))
    , m_iconWorktree(":/small/worktree")
{
    setItemDelegate(new QGitBranchTreeItemDelegate(this));
}

void QGitBranchTreeWidget::populateBranchesAndTags(
    const QList<QGitBranch> &branches,
    const QList<QGitTag> &tags,
    const QList<QGitSubmodule> &submodules,
    const QList<QGitWorktree> &worktrees,
    const QString &currentBranch)
{
    DesiredItem itemFileStatus;
    itemFileStatus.key = QStringLiteral("FileStatus");
    itemFileStatus.text = tr("File Status");

    DesiredItem itemWorkingCopy;
    itemWorkingCopy.key = QStringLiteral("WorkingCopy");
    itemWorkingCopy.text = tr("Working Copy");
    itemWorkingCopy.icon = m_iconWhiteCheckbox;
    itemWorkingCopy.data.insert(Qt::UserRole + 2, QStringLiteral("WorkingCopy"));
    itemFileStatus.children.append(itemWorkingCopy);

    DesiredItem itemLocalBranches;
    itemLocalBranches.key = QStringLiteral("Branches");
    itemLocalBranches.text = tr("Branches");

    DesiredItem itemTags;
    itemTags.key = QStringLiteral("Tags");
    itemTags.text = tr("Tags");

    DesiredItem itemRemoteBranches;
    itemRemoteBranches.key = QStringLiteral("Remotes");
    itemRemoteBranches.text = tr("Remotes");
    itemRemoteBranches.data.insert(Qt::UserRole + 2, QStringLiteral("RemotesHeader"));

    QList<QGitBranch> sortedBranches = branches;
    std::sort(sortedBranches.begin(), sortedBranches.end(), [](const QGitBranch &a, const QGitBranch &b) {
        return a.time() < b.time();
    });

    for (const auto &branch : sortedBranches)
    {
        QStringList branchItems = branch.name().split('/');

        if (branch.type() & GIT_BRANCH_LOCAL)
        {
            if ((branchItems.size() >= 3) && (branchItems[0] == QStringLiteral("refs")) && (branchItems[1] == QStringLiteral("heads")))
            {
                QList<DesiredItem> *currentList = &itemLocalBranches.children;

                for (int depth = 2; depth < branchItems.size(); depth++)
                {
                    const QString &name = branchItems.at(depth);
                    DesiredItem *existing = findChildByKey(*currentList, name);

                    if (existing)
                    {
                        currentList = &existing->children;
                    }
                    else
                    {
                        const bool isLeaf = (depth == branchItems.size() - 1);
                        QString displayName = name;
                        if (isLeaf && (branch.ahead() > 0 || branch.behind() > 0))
                        {
                            QStringList indicators;
                            if (branch.ahead() > 0) {
                                indicators << QStringLiteral("↑%1").arg(branch.ahead());
                            }
                            if (branch.behind() > 0) {
                                indicators << QStringLiteral("↓%1").arg(branch.behind());
                            }
                            displayName += QStringLiteral(" [%1]").arg(indicators.join(QStringLiteral(" ")));
                        }

                        DesiredItem child;
                        child.key = name;
                        child.text = displayName;
                        if (isLeaf) {
                            child.data.insert(Qt::UserRole, branch.hash());
                            child.data.insert(Qt::UserRole + 1, branchItems.mid(2).join('/'));
                            child.data.insert(Qt::UserRole + 2, QStringLiteral("LocalBranch"));
                            child.isEditable = true;
                        } else {
                            child.data.insert(Qt::UserRole + 2, QStringLiteral("Folder"));
                        }

                        const bool isCurrent = isLeaf && (branchItems.mid(2).join('/') == currentBranch);
                        if (isCurrent) {
                            child.isBold = true;
                            child.icon = m_iconCurrentBranch;
                        } else {
                            child.icon = m_iconBranch;
                        }

                        currentList->append(child);
                        currentList = &currentList->last().children;
                    }
                }
            }
        }
        if (branch.type() & GIT_BRANCH_REMOTE)
        {
            if ((branchItems.size() >= 4) && (branchItems[0] == QStringLiteral("refs")) && (branchItems[1] == QStringLiteral("remotes")))
            {
                QList<DesiredItem> *currentList = &itemRemoteBranches.children;

                for (int depth = 2; depth < branchItems.size(); depth++)
                {
                    const QString &name = branchItems.at(depth);
                    DesiredItem *existing = findChildByKey(*currentList, name);

                    if (existing)
                    {
                        currentList = &existing->children;
                    }
                    else
                    {
                        DesiredItem child;
                        child.key = name;
                        child.text = name;
                        if (depth == 2)
                        {
                            child.data.insert(Qt::UserRole + 1, name);
                            child.data.insert(Qt::UserRole + 2, QStringLiteral("Remote"));
                            child.icon = m_iconRemote;
                        }
                        else
                        {
                            child.icon = m_iconRemoteBranch;
                            if (depth == branchItems.size() - 1) {
                                child.data.insert(Qt::UserRole, branch.hash());
                                child.data.insert(Qt::UserRole + 1, branchItems.mid(2).join('/'));
                                child.data.insert(Qt::UserRole + 2, QStringLiteral("RemoteBranch"));
                            } else {
                                child.data.insert(Qt::UserRole + 2, QStringLiteral("Folder"));
                            }
                        }

                        currentList->append(child);
                        currentList = &currentList->last().children;
                    }
                }
            }
        }
    }

    QList<QGitTag> sortedTags = tags;
    std::sort(sortedTags.begin(), sortedTags.end(), [](const QGitTag &a, const QGitTag &b) {
        return a.time() > b.time();
    });

    for (const auto &tag : sortedTags)
    {
        DesiredItem child;
        child.key = tag.name();
        child.text = tag.name();
        child.data.insert(Qt::UserRole, tag.hash());
        child.data.insert(Qt::UserRole + 1, tag.name());
        child.data.insert(Qt::UserRole + 2, QStringLiteral("Tag"));
        child.isEditable = !tag.name().isEmpty();
        child.icon = m_iconTag;
        itemTags.children.append(child);
    }

    DesiredItem itemSubmodules;
    itemSubmodules.key = QStringLiteral("Submodules");
    itemSubmodules.text = tr("Submodules");
    itemSubmodules.data.insert(Qt::UserRole + 2, QStringLiteral("SubmodulesHeader"));
    for (const auto &sub : submodules) {
        constexpr unsigned int commitChangedMask =
            GIT_SUBMODULE_STATUS_WD_MODIFIED | GIT_SUBMODULE_STATUS_INDEX_MODIFIED |
            GIT_SUBMODULE_STATUS_INDEX_ADDED | GIT_SUBMODULE_STATUS_INDEX_DELETED |
            GIT_SUBMODULE_STATUS_WD_ADDED | GIT_SUBMODULE_STATUS_WD_DELETED;
        constexpr unsigned int dirtyWorktreeMask =
            GIT_SUBMODULE_STATUS_WD_INDEX_MODIFIED | GIT_SUBMODULE_STATUS_WD_WD_MODIFIED |
            GIT_SUBMODULE_STATUS_WD_UNTRACKED;

        const bool uninitialized = (sub.status & GIT_SUBMODULE_STATUS_WD_UNINITIALIZED) != 0;
        const bool commitChanged = (sub.status & commitChangedMask) != 0;
        const bool dirtyWorktree = (sub.status & dirtyWorktreeMask) != 0;

        QString statusText;
        QString toolTip = sub.path;
        const QIcon *icon = &m_iconSubmodule;

        if (uninitialized) {
            statusText = tr(" [Uninitialized]");
            toolTip = tr("%1\nSubmodule is not initialized").arg(sub.path);
            icon = &m_iconSubmoduleUninitialized;
        } else if (commitChanged && dirtyWorktree) {
            statusText = tr(" [Modified, Dirty]");
            toolTip = tr("%1\nSubmodule commit differs from superproject and has uncommitted changes").arg(sub.path);
            icon = &m_iconSubmoduleDirty;
        } else if (commitChanged) {
            statusText = tr(" [Modified]");
            toolTip = tr("%1\nSubmodule commit differs from superproject").arg(sub.path);
            icon = &m_iconSubmoduleCommitChanged;
        } else if (dirtyWorktree) {
            statusText = tr(" [Dirty]");
            toolTip = tr("%1\nSubmodule has uncommitted changes in its working directory").arg(sub.path);
            icon = &m_iconSubmoduleDirty;
        }

        DesiredItem child;
        child.key = sub.name;
        child.text = sub.name + statusText;
        child.data.insert(Qt::UserRole, sub.name);
        child.data.insert(Qt::UserRole + 1, sub.path);
        child.data.insert(Qt::UserRole + 2, QStringLiteral("Submodule"));
        child.icon = *icon;
        child.toolTip = toolTip;
        itemSubmodules.children.append(child);
    }

    DesiredItem itemWorktrees;
    itemWorktrees.key = QStringLiteral("Worktrees");
    itemWorktrees.text = tr("Worktrees");
    itemWorktrees.data.insert(Qt::UserRole + 2, QStringLiteral("WorktreesHeader"));
    for (const auto &wt : worktrees) {
        if (wt.isMain())
            continue;
        QString label = wt.name();
        if (!wt.branch().isEmpty())
            label += QStringLiteral(" [%1]").arg(wt.branch());
        if (wt.isLocked())
            label += tr(" [Locked]");

        DesiredItem child;
        child.key = wt.path();
        child.text = label;
        child.data.insert(Qt::UserRole,     wt.path());
        child.data.insert(Qt::UserRole + 1, wt.branch());
        child.data.insert(Qt::UserRole + 2, QStringLiteral("Worktree"));
        child.data.insert(Qt::UserRole + 3, wt.name());
        child.data.insert(Qt::UserRole + 4, wt.isLocked());
        child.icon = m_iconWorktree;
        itemWorktrees.children.append(child);
    }

    QList<DesiredItem> desiredTopLevel;
    desiredTopLevel.append(itemFileStatus);
    desiredTopLevel.append(itemLocalBranches);
    desiredTopLevel.append(itemTags);
    desiredTopLevel.append(itemRemoteBranches);
    if (!itemSubmodules.children.isEmpty())
        desiredTopLevel.append(itemSubmodules);
    if (!itemWorktrees.children.isEmpty())
        desiredTopLevel.append(itemWorktrees);

    const QSignalBlocker blocker(this);
    syncTreeItems(invisibleRootItem(), desiredTopLevel, {QStringLiteral("Stashes")});
}

void QGitBranchTreeWidget::populateStashes(const QStringList &stashes)
{
    const QSignalBlocker blocker(this);

    QTreeWidgetItem *stashGroupItem = nullptr;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        if (topLevelItem(i)->data(0, ItemKeyRole).toString() == QStringLiteral("Stashes")) {
            stashGroupItem = topLevelItem(i);
            break;
        }
    }

    if (stashes.isEmpty())
    {
        if (stashGroupItem) {
            delete takeTopLevelItem(indexOfTopLevelItem(stashGroupItem));
        }
        return;
    }

    if (!stashGroupItem)
    {
        stashGroupItem = new QTreeWidgetItem(QStringList() << tr("Stashes"));
        stashGroupItem->setData(0, ItemKeyRole, QStringLiteral("Stashes"));
        addTopLevelItem(stashGroupItem);
        stashGroupItem->setExpanded(true);
    }

    QList<DesiredItem> desiredStashes;
    desiredStashes.reserve(stashes.size());
    for (int i = 0; i < stashes.size(); ++i)
    {
        const QString &stash = stashes.at(i);
        const QString stashRef = QStringLiteral("stash@{%1}").arg(i);
        DesiredItem item;
        item.key = stashRef;
        item.text = stash;
        item.icon = m_iconStash;
        item.data.insert(Qt::UserRole, stashRef);
        item.data.insert(Qt::UserRole + 1, stash);
        item.data.insert(Qt::UserRole + 2, QStringLiteral("Stash"));
        desiredStashes.append(item);
    }

    syncTreeItems(stashGroupItem, desiredStashes);
}
