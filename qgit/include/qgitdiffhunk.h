#pragma once

#include "qgitdiffline.h"

#include <QString>
#include <QList>


class QGitDiffHunk
{
public:
    QGitDiffHunk();
    QGitDiffHunk(const QString &header, int new_lines, int new_start, int old_lines, int old_start, const QList<QGitDiffLine> &lines);
    QGitDiffHunk(const QGitDiffHunk &other);

    QGitDiffHunk &operator=(QGitDiffHunk &&other);
    QGitDiffHunk &operator=(const QGitDiffHunk &other);

    QString header() const;
    int new_lines() const;
    int new_start() const;
    int old_lines() const;
    int old_start() const;
    QList<QGitDiffLine> lines() const;

private:
    QString m_header;
    int m_new_lines = 0;
    int m_new_start = 0;
    int m_old_lines = 0;
    int m_old_start = 0;
    QList<QGitDiffLine> m_lines;
};
