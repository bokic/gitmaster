#pragma once

#include "qgitdifffileitem.h"
#include "qgitdiffhunk.h"
#include <git2.h>

#include <QList>


class QGitDiffFile
{
     friend class QGitCommitDiffParent;

public:
    QGitDiffFile();
    QGitDiffFile(const QGitDiffFileItem &new_file, const QGitDiffFileItem &old_file, uint32_t flags, int nfiles, int simularity, int status, const QList<QGitDiffHunk> &hunks);
    QGitDiffFile(const QGitDiffFile &other);

    QGitDiffFile &operator=(QGitDiffFile &&other);
    QGitDiffFile &operator=(const QGitDiffFile &other);
    bool operator==(const git_diff_delta *other);

    QGitDiffFileItem new_file() const;
    QGitDiffFileItem old_file() const;
    uint32_t flags() const;
    int nfiles() const;
    int simularity() const;
    int status() const;
    QList<QGitDiffHunk> hunks() const;

    void addHunk(const QGitDiffHunk &hunk);

private:
    QGitDiffFileItem m_new_file;
    QGitDiffFileItem m_old_file;
    uint32_t m_flags = 0;
    int m_nfiles = 0;
    int m_simularity = 0;
    int m_status = 0;
    QList<QGitDiffHunk> m_hunks;
};
