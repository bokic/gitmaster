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
    QGitDiffFile(const QGitDiffFile &other) = default;
    QGitDiffFile(QGitDiffFile &&other) noexcept = default;
    ~QGitDiffFile() = default;

    QGitDiffFile &operator=(QGitDiffFile &&other) noexcept = default;
    QGitDiffFile &operator=(const QGitDiffFile &other) = default;
    bool operator==(const git_diff_delta *other) const;
    bool operator==(const QGitDiffFile &other) const;

    const QGitDiffFileItem &new_file() const;
    const QGitDiffFileItem &old_file() const;
    uint32_t flags() const;
    int nfiles() const;
    int simularity() const;
    int status() const;
    const QList<QGitDiffHunk> &hunks() const;
    const QList<QGitDiffBinary> &binaries() const;

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
