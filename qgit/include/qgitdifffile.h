#pragma once

#include "qgitdifffileitem.h"
#include "qgitdiffhunk.h"

#include <QList>


class QGitDiffFile
{
public:
    QGitDiffFile();
    QGitDiffFile(const QGitDiffFileItem &new_file, const QGitDiffFileItem &old_file, int flags, int nfiles, int simularity, int status, const QList<QGitDiffHunk> &hunks);
    QGitDiffFile(const QGitDiffFile &other);

    QGitDiffFile &operator=(QGitDiffFile &&other);
    QGitDiffFile &operator=(const QGitDiffFile &other);

    QGitDiffFileItem new_file() const;
    QGitDiffFileItem old_file() const;
    int flags() const;
    int nfiles() const;
    int simularity() const;
    int status() const;
    QList<QGitDiffHunk> hunks() const;

private:
    QGitDiffFileItem m_new_file;
    QGitDiffFileItem m_old_file;
    int m_flags = 0;
    int m_nfiles = 0;
    int m_simularity = 0;
    int m_status = 0;
    QList<QGitDiffHunk> m_hunks;
};
