#include "qloghistorytablewidget.h"


QLogHistoryTableWidget::QLogHistoryTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
}

void QLogHistoryTableWidget::addCommit(const QGitCommit &commit)
{
    if (columnCount() == 4)
    {
        int row = rowCount();
        insertRow(row);

        QTableWidgetItem *item = nullptr;

        item = new QTableWidgetItem(commit.message().section('\n', 0, 0));
        item->setData(Qt::UserRole, commit.message());
        item->setData(Qt::UserRole + 1, commit.id());
        setItem(row, 0, item);

        item = new QTableWidgetItem(commit.time().toString());
        setItem(row, 1, item);

        item = new QTableWidgetItem(QString("%1 <%2>").arg(commit.author().name(), commit.author().email()));
        item->setData(Qt::UserRole, commit.author().name());
        item->setData(Qt::UserRole + 1, commit.author().email());
        setItem(row, 2, item);

        item = new QTableWidgetItem(commit.id().left(7));
        item->setData(Qt::UserRole, commit.id());
        setItem(row, 3, item);
        return;
    }

    QString commitId = commit.id().toLower();
    int row = rowCount();    

    // Reset lane tracking if the table is cleared/empty
    if (row == 0)
    {
        m_hashIndex.clear();
    }

    // Keep track of active lanes at the top of this row (BEFORE we process/modify for the current commit)
    QList<int> activeLanesAtTop;
    for (int i = 0; i < m_hashIndex.size(); ++i)
    {
        if (!m_hashIndex[i].isEmpty())
        {
            activeLanesAtTop.append(i);
        }
    }

    // Find all lanes that currently track this commitId case-insensitively
    QList<int> mergingLanes;
    for (int i = 0; i < m_hashIndex.size(); ++i)
    {
        if (m_hashIndex[i].toLower() == commitId)
        {
            mergingLanes.append(i);
        }
    }

    int commitLane = -1;
    QList<QVariantList> childConnections; // Connections from child lanes (top) to node (center)

    if (!mergingLanes.isEmpty())
    {
        // Node is drawn at the first lane where it is found
        commitLane = mergingLanes.first();

        // All other lanes tracking this commit merge into the node lane from top
        for (int i = 1; i < mergingLanes.size(); ++i)
        {
            int otherLane = mergingLanes[i];
            childConnections.append(QVariantList{otherLane, commitLane});
        }
    }
    else
    {
        // Start a new lane
        m_hashIndex.append(commitId);
        commitLane = m_hashIndex.length() - 1;
    }

    // Determine parents and build connections
    QList<QVariantList> parentConnections;
    QList<QString> nextHashIndex = m_hashIndex;

    // Clear all merging lanes from nextHashIndex so they don't continue down
    for (int lane : mergingLanes)
    {
        nextHashIndex[lane] = "";
    }

    if (commit.parents().count() > 0)
    {
        // First parent continues in our commit's main lane
        const QString firstParent = commit.parents().at(0).commitHash().toLower();

        nextHashIndex[commitLane] = firstParent;
        parentConnections.append(QVariantList{commitLane, commitLane});

        // Other parents (merges)
        for (int c = 1; c < commit.parents().count(); c++)
        {
            const QString otherParent = commit.parents().at(c).commitHash().toLower();
            int parentIdx = -1;
            for (int i = 0; i < nextHashIndex.size(); ++i)
            {
                if (nextHashIndex[i].toLower() == otherParent)
                {
                    parentIdx = i;
                    break;
                }
            }

            if (parentIdx < 0)
            {
                // Find empty slot or append
                int emptySlot = -1;
                for (int i = 0; i < nextHashIndex.size(); ++i)
                {
                    if (nextHashIndex[i].isEmpty())
                    {
                        emptySlot = i;
                        break;
                    }
                }

                if (emptySlot >= 0)
                {
                    nextHashIndex[emptySlot] = otherParent;
                    parentConnections.append(QVariantList{commitLane, emptySlot});
                }
                else
                {
                    nextHashIndex.append(otherParent);
                    parentConnections.append(QVariantList{commitLane, nextHashIndex.length() - 1});
                }
            }
            else
            {
                parentConnections.append(QVariantList{commitLane, parentIdx});
            }
        }
    }
    else
    {
        // Root commit, lane terminates
        nextHashIndex[commitLane] = "";
    }

    // Clean trailing empty lanes
    while (!nextHashIndex.isEmpty() && nextHashIndex.last().isEmpty())
    {
        nextHashIndex.removeLast();
    }

    // Passing lines are lanes active at top that:
    // 1. are not commitLane
    // 2. are not in mergingLanes (since those terminate at this node)
    QList<int> passingLines;
    for (int lane : activeLanesAtTop)
    {
        if (lane != commitLane && !mergingLanes.contains(lane))
        {
            passingLines.append(lane);
        }
    }

    // Update m_hashIndex for the next row
    m_hashIndex = nextHashIndex;

    // Pack into data for column 0 QTableWidgetItem
    QList<QVariant> data;
    data.append(commitLane);

    QList<QVariant> passingLanesVar;
    for (int lane : passingLines)
    {
        passingLanesVar.append(lane);
    }
    data.append(QVariant(passingLanesVar));

    QList<QVariant> parentConnsVar;
    for (const auto &conn : parentConnections)
    {
        if (conn.size() >= 2)
        {
            parentConnsVar.append(conn.at(0).toInt());
            parentConnsVar.append(conn.at(1).toInt());
        }
    }
    data.append(QVariant(parentConnsVar));
 
    QList<QVariant> activeLanesVar;
    for (int lane : activeLanesAtTop)
    {
        activeLanesVar.append(lane);
    }
    data.append(QVariant(activeLanesVar));
 
    QList<QVariant> childConnsVar;
    for (const auto &conn : childConnections)
    {
        if (conn.size() >= 2)
        {
            childConnsVar.append(conn.at(0).toInt());
            childConnsVar.append(conn.at(1).toInt());
        }
    }
    data.append(QVariant(childConnsVar));

    insertRow(row);

    QTableWidgetItem *item = nullptr;

    item = new QTableWidgetItem();
    item->setData(Qt::UserRole, data);
    setItem(row, 0, item);

    item = new QTableWidgetItem(commit.message().section('\n', 0, 0));
    item->setData(Qt::UserRole, commit.message());
    item->setData(Qt::UserRole + 1, commit.id());
    setItem(row, 1, item);

    item = new QTableWidgetItem(commit.time().toString());
    setItem(row, 2, item);

    item = new QTableWidgetItem(QString("%1 <%2>").arg(commit.author().name(), commit.author().email()));
    item->setData(Qt::UserRole, commit.author().name());
    item->setData(Qt::UserRole + 1, commit.author().email());
    setItem(row, 3, item);

    item = new QTableWidgetItem(commit.id().left(7));
    item->setData(Qt::UserRole, commit.id());
    setItem(row, 4, item);
}

bool QLogHistoryTableWidget::selectCommit(const QString &hash)
{
    for (int row = 0; row < rowCount(); ++row)
    {
        int col = (columnCount() == 4) ? 3 : 4;
        QTableWidgetItem *item = this->item(row, col);
        if (item && item->data(Qt::UserRole).toString().compare(hash, Qt::CaseInsensitive) == 0)
        {
            this->selectRow(row);
            this->scrollToItem(item);
            return true;
        }
    }
    return false;
}

void QLogHistoryTableWidget::setReferences(const QList<QGitBranch> &branches, const QList<QGitTag> &tags, const QString &currentBranch)
{
    m_refMap.clear();

    for (const auto &branch : branches)
    {
        QString refName = branch.name();
        QGitRef::Type refType = QGitRef::LocalBranch;

        if (branch.type() & GIT_BRANCH_LOCAL)
        {
            if (refName.startsWith(QStringLiteral("refs/heads/")))
            {
                refName = refName.mid(11);
            }
            if (refName == currentBranch)
            {
                refType = QGitRef::CurrentBranch;
            }
            else
            {
                refType = QGitRef::LocalBranch;
            }
        }
        else if (branch.type() & GIT_BRANCH_REMOTE)
        {
            if (refName.startsWith(QStringLiteral("refs/remotes/")))
            {
                refName = refName.mid(13);
            }
            refType = QGitRef::RemoteBranch;
        }

        QGitRef refObj{refName, refType};
        m_refMap[branch.hash().toUpper()].append(refObj);
        m_refMap[branch.hash().toLower()].append(refObj);
    }

    for (const auto &tag : tags)
    {
        QString refName = tag.name();
        if (refName.startsWith(QStringLiteral("refs/tags/")))
        {
            refName = refName.mid(10);
        }
        QGitRef refObj{refName, QGitRef::Tag};
        m_refMap[tag.hash().toUpper()].append(refObj);
        m_refMap[tag.hash().toLower()].append(refObj);
    }

    viewport()->update();
}

QList<QGitRef> QLogHistoryTableWidget::getReferences(const QString &sha) const
{
    if (m_refMap.contains(sha.toUpper()))
    {
        return m_refMap.value(sha.toUpper());
    }
    return m_refMap.value(sha.toLower());
}

