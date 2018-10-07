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
    QGitCommitDiffParent(QByteArray commitHash);
    QGitCommitDiffParent() = default;
    QGitCommitDiffParent(const QGitCommitDiffParent &other) = default;

    QGitCommitDiffParent &operator=(QGitCommitDiffParent &&other) noexcept;
    QGitCommitDiffParent &operator=(const QGitCommitDiffParent &other) = default;

    QByteArray commitHash() const;
    QList<QGitDiffFile> files() const;

    void addFile(const git_diff_delta *delta);
    void addBinary(const git_diff_delta *delta, const git_diff_binary *binary);
    void addHunk(const git_diff_delta *delta, const git_diff_hunk *hunk);
    void addLine(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line);

private:
    bool hasFile(const git_diff_delta *delta);

    QByteArray m_commitHash;
    QList<QGitDiffFile> m_files;
};
