#pragma once

#include "qgitdifffileitem.h"
#include "qgitdiffbinary.h"
#include "qgitdiffhunk.h"
#include <git2.h>

#include <QList>


class QGitDiffFile
{
     friend class QGitCommitDiffParent;

public:
    QGitDiffFile() = default;
    QGitDiffFile(const git_diff_delta *file);
    QGitDiffFile(const QGitDiffFile &other);
    QGitDiffFile(QGitDiffFile &&other) noexcept;

    QGitDiffFile &operator=(QGitDiffFile &&other) noexcept;
    QGitDiffFile &operator=(const QGitDiffFile &other);
    bool operator==(const git_diff_delta *other);
    bool operator==(const QGitDiffFile &other);

    QGitDiffFileItem new_file() const;
    QGitDiffFileItem old_file() const;
    uint32_t flags() const;
    int nfiles() const;
    int simularity() const;
    int status() const;
    QList<QGitDiffHunk> hunks() const;
    QList<QGitDiffBinary> binaries() const;

    void addHunk(const QGitDiffHunk &hunk);
    void addBinary(const QGitDiffBinary &binary);

private:
    QGitDiffFileItem m_new_file;
    QGitDiffFileItem m_old_file;
    uint32_t m_flags = 0;
    int m_nfiles = 0;
    int m_simularity = 0;
    int m_status = 0;
    QList<QGitDiffHunk> m_hunks;
    QList<QGitDiffBinary> m_binary;
};
