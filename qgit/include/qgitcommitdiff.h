#pragma once

#include "qgitdifffile.h"

#include <QString>
#include <QList>


class QGitCommitDiff
{
public:
    QGitCommitDiff();
    QGitCommitDiff(QString parentHash, QList<QGitDiffFile> files);
    QGitCommitDiff(const QGitCommitDiff &other);

    QGitCommitDiff &operator=(QGitCommitDiff &&other);
    QGitCommitDiff &operator=(const QGitCommitDiff &other);

    QString parentHash() const;
    QList<QGitDiffFile> files() const;

private:
    QString m_parentHash;
    QList<QGitDiffFile> m_files;
};
