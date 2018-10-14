#pragma once

#include "git2.h"
#include "qgitdifffile.h"
#include "qgitdiffhunk.h"

#include <QByteArray>
#include <QString>
#include <QList>


class QGitCommitDiffParent
{
public:
    QGitCommitDiffParent() = default;
    QGitCommitDiffParent(const QByteArray &commitHash);
    QGitCommitDiffParent(const QGitCommitDiffParent &other) = default;

    QGitCommitDiffParent &operator=(QGitCommitDiffParent &&other) noexcept;
    QGitCommitDiffParent &operator=(const QGitCommitDiffParent &other) = default;

    QByteArray commitHash() const;
    QList<QGitDiffFile> files() const;

    void addFile(const git_diff_delta *delta);

private:
    QByteArray m_commitHash;
    QList<QGitDiffFile> m_files;
};
