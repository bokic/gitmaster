// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qloghistorytablewidget.h"
#include <QResizeEvent>
#include <QSignalBlocker>


QLogHistoryTableWidget::QLogHistoryTableWidget(QWidget *parent)
    : QTableWidget(parent)
{
}

void QLogHistoryTableWidget::clearCommits()
{
    clearContents();
    setRowCount(0);
    m_commits.clear();
    m_hashIndex.clear();
}

QList<QVariant> QLogHistoryTableWidget::computeGraphData(const QGitCommit &commit, QStringList &hashIndex) const
{
    const QString commitId = commit.id().toLower();

    // Keep track of active lanes at the top of this row (BEFORE we process/modify for the current commit)
    QList<int> activeLanesAtTop;
    for (int i = 0; i < hashIndex.size(); ++i)
    {
        if (!hashIndex[i].isEmpty())
        {
            activeLanesAtTop.append(i);
        }
    }

    // Find all lanes that currently track this commitId case-insensitively
    QList<int> mergingLanes;
    for (int i = 0; i < hashIndex.size(); ++i)
    {
        if (hashIndex[i].toLower() == commitId)
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
        hashIndex.append(commitId);
        commitLane = static_cast<int>(hashIndex.size()) - 1;
    }

    // Determine parents and build connections
    QList<QVariantList> parentConnections;
    QStringList nextHashIndex = hashIndex;

    // Clear all merging lanes from nextHashIndex so they don't continue down
    for (int lane : mergingLanes)
    {
        nextHashIndex[lane] = "";
    }

    if (commit.parents().size() > 0)
    {
        // First parent continues in our commit's main lane
        const QString firstParent = commit.parents().at(0).commitHash().toLower();

        nextHashIndex[commitLane] = firstParent;
        parentConnections.append(QVariantList{commitLane, commitLane});

        // Other parents (merges)
        for (int c = 1; c < commit.parents().size(); c++)
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
                    parentConnections.append(QVariantList{commitLane, nextHashIndex.size() - 1});
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

    // Update hashIndex for the next row
    hashIndex = nextHashIndex;

    // Pack into data for column 0 QTableWidgetItem
    QList<QVariant> graphData;
    graphData.append(commitLane);

    QList<QVariant> passingLanesVar;
    for (int lane : passingLines)
    {
        passingLanesVar.append(lane);
    }
    graphData.append(QVariant(passingLanesVar));

    QList<QVariant> parentConnsVar;
    for (const auto &conn : parentConnections)
    {
        if (conn.size() >= 2)
        {
            parentConnsVar.append(conn.at(0).toInt());
            parentConnsVar.append(conn.at(1).toInt());
        }
    }
    graphData.append(QVariant(parentConnsVar));

    QList<QVariant> activeLanesVar;
    for (int lane : activeLanesAtTop)
    {
        activeLanesVar.append(lane);
    }
    graphData.append(QVariant(activeLanesVar));

    QList<QVariant> childConnsVar;
    for (const auto &conn : childConnections)
    {
        if (conn.size() >= 2)
        {
            childConnsVar.append(conn.at(0).toInt());
            childConnsVar.append(conn.at(1).toInt());
        }
    }
    graphData.append(QVariant(childConnsVar));

    return graphData;
}

void QLogHistoryTableWidget::populateCommitRow(int row, const QGitCommit &commit)
{
    QTableWidgetItem *item = nullptr;

    if (columnCount() == 4)
    {
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

    item = new QTableWidgetItem();
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

void QLogHistoryTableWidget::recalculateGraph()
{
    if (columnCount() == 4)
        return;

    QStringList hashIndex;
    for (int row = 0; row < m_commits.size(); ++row)
    {
        const QList<QVariant> graphData = computeGraphData(m_commits.at(row), hashIndex);
        QTableWidgetItem *graphItem = item(row, 0);
        if (!graphItem) {
            graphItem = new QTableWidgetItem();
            setItem(row, 0, graphItem);
        }
        graphItem->setData(Qt::UserRole, graphData);
    }

    m_hashIndex = hashIndex;
    viewport()->update();
}

void QLogHistoryTableWidget::syncCommits(const QList<QGitCommit> &commits)
{
    if (m_commits.isEmpty()) {
        for (const auto &commit : commits) {
            addCommit(commit);
        }
        return;
    }

    int oldPrefixCount = static_cast<int>(m_commits.size());
    if (columnCount() != 4) {
        QHash<QString, int> oldIndexById;
        oldIndexById.reserve(m_commits.size());
        for (int i = 0; i < m_commits.size(); ++i) {
            oldIndexById.insert(m_commits.at(i).id().toLower(), i);
        }

        int anchorNew = -1;
        int anchorOld = -1;
        for (int i = static_cast<int>(commits.size()) - 1; i >= 0; --i) {
            const QString id = commits.at(i).id().toLower();
            if (oldIndexById.contains(id)) {
                anchorNew = i;
                anchorOld = oldIndexById.value(id);
                break;
            }
        }

        if (anchorNew == static_cast<int>(commits.size()) - 1 && anchorOld >= 0) {
            oldPrefixCount = anchorOld + 1;
        }
    }

    QString selectedHash;
    const int prevRow = currentRow();
    if (prevRow >= 0 && prevRow < m_commits.size()) {
        selectedHash = m_commits.at(prevRow).id();
    }

    const QSignalBlocker blocker(this);

    QSet<QString> newIds;
    newIds.reserve(commits.size());
    for (const auto &commit : commits) {
        newIds.insert(commit.id().toLower());
    }

    for (int r = oldPrefixCount - 1; r >= 0; --r) {
        if (!newIds.contains(m_commits.at(r).id().toLower())) {
            removeRow(r);
            m_commits.removeAt(r);
            --oldPrefixCount;
        }
    }

    for (int i = 0; i < commits.size(); ++i) {
        const QGitCommit &commit = commits.at(i);
        const QString id = commit.id().toLower();

        int matchIdx = -1;
        for (int r = i; r < oldPrefixCount; ++r) {
            if (m_commits.at(r).id().toLower() == id) {
                matchIdx = r;
                break;
            }
        }

        if (matchIdx >= 0) {
            if (matchIdx != i) {
                QList<QTableWidgetItem *> rowItems;
                rowItems.reserve(columnCount());
                for (int c = 0; c < columnCount(); ++c) {
                    rowItems.append(takeItem(matchIdx, c));
                }
                removeRow(matchIdx);
                insertRow(i);
                for (int c = 0; c < columnCount(); ++c) {
                    setItem(i, c, rowItems.at(c));
                }
                m_commits.move(matchIdx, i);
            }
            m_commits[i] = commit;
        } else {
            insertRow(i);
            populateCommitRow(i, commit);
            m_commits.insert(i, commit);
            ++oldPrefixCount;
        }
    }

    recalculateGraph();

    if (!selectedHash.isEmpty()) {
        for (int r = 0; r < m_commits.size(); ++r) {
            if (m_commits.at(r).id().compare(selectedHash, Qt::CaseInsensitive) == 0) {
                setCurrentCell(r, 0, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                break;
            }
        }
    }
}

void QLogHistoryTableWidget::addCommit(const QGitCommit &commit)
{
    int row = rowCount();

    if (columnCount() == 4)
    {
        if (row == 0) {
            m_commits.clear();
        }
        insertRow(row);
        populateCommitRow(row, commit);
        m_commits.append(commit);
        return;
    }

    // Reset lane tracking if the table is cleared/empty
    if (row == 0)
    {
        m_hashIndex.clear();
        m_commits.clear();
    }

    const QList<QVariant> graphData = computeGraphData(commit, m_hashIndex);

    insertRow(row);
    populateCommitRow(row, commit);
    if (QTableWidgetItem *graphItem = item(row, 0)) {
        graphItem->setData(Qt::UserRole, graphData);
    }
    m_commits.append(commit);
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

        QGitRef refObj = {refName, refType};
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
        QGitRef refObj = {refName, QGitRef::Tag};
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

void QLogHistoryTableWidget::setHighlightedCommits(const QSet<QString> &shas)
{
    m_highlightedCommits.clear();
    for (const auto &sha : shas)
    {
        m_highlightedCommits.insert(sha.toLower());
    }
    viewport()->update();
}

bool QLogHistoryTableWidget::isCommitHighlighted(const QString &sha) const
{
    return m_highlightedCommits.contains(sha.toLower());
}

void QLogHistoryTableWidget::setAutoResizeColumn(int column)
{
    m_autoResizeColumn = column;
}

void QLogHistoryTableWidget::resizeEvent(QResizeEvent *event)
{
    QTableWidget::resizeEvent(event);

    if (m_autoResizeColumn >= 0 && m_autoResizeColumn < columnCount() && !isColumnHidden(m_autoResizeColumn))
    {
        int otherWidth = 0;
        for (int c = 0; c < columnCount(); ++c)
        {
            if (c != m_autoResizeColumn && !isColumnHidden(c))
            {
                otherWidth += columnWidth(c);
            }
        }
        const int targetWidth = qMax(100, viewport()->width() - otherWidth);
        if (columnWidth(m_autoResizeColumn) != targetWidth)
        {
            setColumnWidth(m_autoResizeColumn, targetWidth);
        }
    }
}

