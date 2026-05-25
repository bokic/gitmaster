#pragma once

#include "qgitcommit.h"
#include "qgitbranch.h"
#include "qgittag.h"

#include <QTableWidget>
#include <QString>
#include <QList>
#include <QHash>


struct QGitRef {
    enum Type {
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
    bool selectCommit(const QString &hash);

    void setReferences(const QList<QGitBranch> &branches, const QList<QGitTag> &tags, const QString &currentBranch);
    QList<QGitRef> getReferences(const QString &sha) const;

private:
    QList<QString> m_hashIndex;
    QHash<QString, QList<QGitRef>> m_refMap;
};

