#pragma once

#include "qgitcommit.h"
#include "qgitbranch.h"
#include "qgittag.h"

#include <QTableWidget>
#include <QString>
#include <QList>
#include <QHash>
#include <QSet>


struct QGitRef {
    enum Type : uint8_t {
        LocalBranch,
        RemoteBranch,
        Tag,
        CurrentBranch
    };
    QString name;
    Type type;
};

class QLogHistoryTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit QLogHistoryTableWidget(QWidget *parent = nullptr);

    void addCommit(const QGitCommit &commit);
    void syncCommits(const QList<QGitCommit> &commits);
    void recalculateGraph();
    bool selectCommit(const QString &hash);
    void clearCommits();

    void setReferences(const QList<QGitBranch> &branches, const QList<QGitTag> &tags, const QString &currentBranch);
    QList<QGitRef> getReferences(const QString &sha) const;

    void setHighlightedCommits(const QSet<QString> &shas);
    bool isCommitHighlighted(const QString &sha) const;

private:
    QList<QVariant> computeGraphData(const QGitCommit &commit, QStringList &hashIndex) const;
    void populateCommitRow(int row, const QGitCommit &commit);

    QList<QGitCommit> m_commits;
    QStringList m_hashIndex;
    QHash<QString, QList<QGitRef>> m_refMap;
    QSet<QString> m_highlightedCommits;
};

