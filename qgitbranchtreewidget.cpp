#include "qgitbranchtreewidget.h"
#include <algorithm>

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
    QList<QTreeWidgetItem *> items;
    QTreeWidgetItem *itemWorkingCopy = new QTreeWidgetItem(QStringList() << tr("Working Copy"));
    QTreeWidgetItem *itemFileStatus = new QTreeWidgetItem(QStringList() << tr("File Status"));
    QTreeWidgetItem *itemLocalBranches = new QTreeWidgetItem(QStringList() << tr("Branches"));
    QTreeWidgetItem *itemTags = new QTreeWidgetItem(QStringList() << tr("Tags"));
    QTreeWidgetItem *itemRemoteBranches = new QTreeWidgetItem(QStringList() << tr("Remotes"));
    itemRemoteBranches->setData(0, Qt::UserRole + 2, QStringLiteral("RemotesHeader"));

    itemWorkingCopy->setData(0, Qt::UserRole + 2, "WorkingCopy");
    itemWorkingCopy->setIcon(0, m_iconWhiteCheckbox);
    itemFileStatus->addChild(itemWorkingCopy);

    QList<QGitBranch> sortedBranches = branches;
    std::sort(sortedBranches.begin(), sortedBranches.end(), [](const QGitBranch &a, const QGitBranch &b) {
        return a.time() < b.time();
    });

    for (const auto &branch : sortedBranches)
    {
        QStringList branchItems = branch.name().split('/');

        if (branch.type() & GIT_BRANCH_LOCAL)
        {
            if ((branchItems.count() >= 3) && (branchItems[0] == QStringLiteral("refs")) && (branchItems[1] == QStringLiteral("heads")))
            {
                QTreeWidgetItem *item = itemLocalBranches;

                for (int depth = 2; depth < branchItems.count(); depth++)
                {
                    const QString &name = branchItems.at(depth);
                    bool found = false;

                    for (int c = 0; c < item->childCount(); c++)
                    {
                        if (item->child(c)->text(0) == name)
                        {
                            item = item->child(c);
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        QString displayName = name;
                        if (depth == branchItems.count() - 1)
                        {
                            if (branch.ahead() > 0 || branch.behind() > 0)
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
                        }

                        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << displayName);
                        child->setData(0, Qt::UserRole, branch.hash());
                        if (depth == branchItems.count() - 1) {
                            child->setData(0, Qt::UserRole + 1, branchItems.mid(2).join('/'));
                            child->setData(0, Qt::UserRole + 2, "LocalBranch");
                            child->setFlags(child->flags() | Qt::ItemIsEditable);
                        }

                        bool isCurrent = false;
                        if (depth == branchItems.count() - 1 && branchItems.mid(2).join('/') == currentBranch) {
                            isCurrent = true;
                        }

                        if (isCurrent)
                        {
                            auto font = child->font(0);
                            font.setBold(true);
                            child->setFont(0, font);
                            child->setIcon(0, m_iconCurrentBranch);
                        }
                        else
                        {
                            child->setIcon(0, m_iconBranch);
                        }

                        item->addChild(child);
                        item = child;
                    }
                }
            }
        }
        if (branch.type() & GIT_BRANCH_REMOTE)
        {
            if ((branchItems.count() >= 4) && (branchItems[0] == QStringLiteral("refs")) && (branchItems[1] == QStringLiteral("remotes")))
            {
                QTreeWidgetItem *item = itemRemoteBranches;

                for (int depth = 2; depth < branchItems.count(); depth++)
                {
                    const QString &name = branchItems.at(depth);
                    bool found = false;

                    for (int c = 0; c < item->childCount(); c++)
                    {
                        if (item->child(c)->text(0) == name)
                        {
                            item = item->child(c);
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << name);
                        child->setData(0, Qt::UserRole, branch.hash());
                        if (depth == 2)
                        {
                            child->setData(0, Qt::UserRole + 1, name);
                            child->setData(0, Qt::UserRole + 2, "Remote");
                            child->setIcon(0, m_iconRemote);
                        }
                        else
                        {
                            child->setIcon(0, m_iconRemoteBranch);
                            if (depth == branchItems.count() - 1) {
                                child->setData(0, Qt::UserRole + 1, branchItems.mid(2).join('/'));
                                child->setData(0, Qt::UserRole + 2, "RemoteBranch");
                            }
                        }

                        item->addChild(child);
                        item = child;
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
        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << tag.name());
        child->setData(0, Qt::UserRole, tag.hash());
        child->setData(0, Qt::UserRole + 1, tag.name());
        child->setData(0, Qt::UserRole + 2, "Tag");
        child->setFlags(child->flags() | Qt::ItemIsEditable);

        child->setIcon(0, m_iconTag);
        itemTags->addChild(child);
    }

    QTreeWidgetItem *itemSubmodules = new QTreeWidgetItem(QStringList() << tr("Submodules"));
    for (const auto &sub : submodules) {
        QString label = sub.name;
        QString statusText;
        if (sub.status & GIT_SUBMODULE_STATUS_WD_UNINITIALIZED) {
            statusText = tr(" [Uninitialized]");
        } else if (sub.status & (GIT_SUBMODULE_STATUS_WD_ADDED | GIT_SUBMODULE_STATUS_WD_DELETED | 
                                 GIT_SUBMODULE_STATUS_WD_MODIFIED | GIT_SUBMODULE_STATUS_WD_INDEX_MODIFIED | 
                                 GIT_SUBMODULE_STATUS_WD_WD_MODIFIED)) {
            statusText = tr(" [Dirty]");
        }

        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << (label + statusText));
        child->setData(0, Qt::UserRole, sub.name);
        child->setData(0, Qt::UserRole + 1, sub.path);
        child->setData(0, Qt::UserRole + 2, QStringLiteral("Submodule"));
        child->setIcon(0, m_iconSubmodule);
        itemSubmodules->addChild(child);
    }

    QTreeWidgetItem *itemWorktrees = new QTreeWidgetItem(QStringList() << tr("Worktrees"));
    itemWorktrees->setData(0, Qt::UserRole + 2, QStringLiteral("WorktreesHeader"));
    for (const auto &wt : worktrees) {
        if (wt.isMain())
            continue;
        QString label = wt.name();
        if (!wt.branch().isEmpty())
            label += QStringLiteral(" [%1]").arg(wt.branch());
        if (wt.isLocked())
            label += tr(" [Locked]");
        QTreeWidgetItem *child = new QTreeWidgetItem(QStringList() << label);
        child->setData(0, Qt::UserRole,     wt.path());
        child->setData(0, Qt::UserRole + 1, wt.branch());
        child->setData(0, Qt::UserRole + 2, QStringLiteral("Worktree"));
        child->setData(0, Qt::UserRole + 3, wt.name());
        child->setData(0, Qt::UserRole + 4, wt.isLocked());
        child->setIcon(0, m_iconWorktree);
        itemWorktrees->addChild(child);
    }

    items.append(itemFileStatus);
    items.append(itemLocalBranches);
    items.append(itemTags);
    items.append(itemRemoteBranches);
    if (itemSubmodules->childCount() > 0) {
        items.append(itemSubmodules);
    } else {
        delete itemSubmodules;
    }
    if (itemWorktrees->childCount() > 0) {
        items.append(itemWorktrees);
    } else {
        delete itemWorktrees;
    }

    clear();
    addTopLevelItems(items);
    expandAll();
}

void QGitBranchTreeWidget::populateStashes(const QStringList &stashes)
{
    QTreeWidgetItem *stashGroupItem = nullptr;
    for (int i = 0; i < topLevelItemCount(); ++i) {
        if (topLevelItem(i)->text(0) == tr("Stashes")) {
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
        addTopLevelItem(stashGroupItem);
    }
    else
    {
        qDeleteAll(stashGroupItem->takeChildren());
    }

    for (const auto &stash : stashes)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << stash);
        item->setIcon(0, m_iconStash);
        item->setData(0, Qt::UserRole + 1, stash);
        item->setData(0, Qt::UserRole + 2, "Stash");
        stashGroupItem->addChild(item);
    }

    expandAll();
}
