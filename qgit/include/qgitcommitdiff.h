#pragma once

#include "qgitcommitdiffparent.h"

#include <QString>
#include <QList>


class QGitCommitDiff
{
public:
    QGitCommitDiff();
    QGitCommitDiff(const QGitCommitDiff &other);

    QGitCommitDiff &operator=(QGitCommitDiff &&other);
    QGitCommitDiff &operator=(const QGitCommitDiff &other);

    void addParent(const QGitCommitDiffParent &parent);
    QList<QGitCommitDiffParent> parents() const;

private:
    QList<QGitCommitDiffParent> m_parents;
};
