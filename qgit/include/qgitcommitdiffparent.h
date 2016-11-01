#pragma once

#include "qgitdifffile.h"

#include <QString>
#include <QList>

class QGitCommitDiffParent
{
public:
    QGitCommitDiffParent(QString commitHash = "");
    QGitCommitDiffParent(const QGitCommitDiffParent &other);

    QGitCommitDiffParent &operator=(QGitCommitDiffParent &&other);
    QGitCommitDiffParent &operator=(const QGitCommitDiffParent &other);

    QString commitHash() const;
    QList<QGitDiffFile> files() const;

private:
    QString m_commitHash;
    QList<QGitDiffFile> m_files;
};
