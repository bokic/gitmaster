#pragma once

#include "qgitdiffline.h"
#include <git2.h>

#include <QString>
#include <QList>


class QGitDiffHunk
{
public:
    QGitDiffHunk() = default;
    QGitDiffHunk(const git_diff_hunk *hunk);
    QGitDiffHunk(const QGitDiffHunk &other) = default;
    QGitDiffHunk(QGitDiffHunk &&other) noexcept;

    QGitDiffHunk &operator=(QGitDiffHunk &&other) noexcept;
    QGitDiffHunk &operator=(const QGitDiffHunk &other) = default;

    QString header() const;
    int new_lines() const;
    int new_start() const;
    int old_lines() const;
    int old_start() const;
    QList<QGitDiffLine> lines() const;

    void addLine(const QGitDiffLine &line);

private:
    QString m_header;
    int m_new_lines = -1;
    int m_new_start = -1;
    int m_old_lines = -1;
    int m_old_start = -1;
    QList<QGitDiffLine> m_lines;
};

bool operator==(const QGitDiffHunk &l, const git_diff_hunk &r);
