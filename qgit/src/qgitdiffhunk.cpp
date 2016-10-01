#include "qgitdiffhunk.h"

#include <QString>
#include <QList>


QGitDiffHunk::QGitDiffHunk()
{
}

QGitDiffHunk::QGitDiffHunk(const QString &header, int new_lines, int new_start, int old_lines, int old_start, const QList<QGitDiffLine> &lines)
    : m_header(header)
    , m_new_lines(new_lines)
    , m_new_start(new_start)
    , m_old_lines(old_lines)
    , m_old_start(old_start)
    , m_lines(lines)
{
}

QGitDiffHunk::QGitDiffHunk(const QGitDiffHunk &other)
    : m_header(other.m_header)
    , m_new_lines(other.m_new_lines)
    , m_new_start(other.m_new_start)
    , m_old_lines(other.m_old_lines)
    , m_old_start(other.m_old_start)
    , m_lines(other.m_lines)
{
}

QGitDiffHunk &QGitDiffHunk::operator=(QGitDiffHunk &&other)
{
    qSwap(m_header, other.m_header);
    qSwap(m_new_lines, other.m_new_lines);
    qSwap(m_new_start, other.m_new_start);
    qSwap(m_old_lines, other.m_old_lines);
    qSwap(m_old_start, other.m_old_start);
    qSwap(m_lines, other.m_lines);

    return *this;
}

QGitDiffHunk &QGitDiffHunk::operator=(const QGitDiffHunk &other)
{
    m_header = other.m_header;
    m_new_lines = other.m_new_lines;
    m_new_start = other.m_new_start;
    m_old_lines = other.m_old_lines;
    m_old_start = other.m_old_start;
    m_lines = other.m_lines;

    return *this;
}

QString QGitDiffHunk::header() const
{
    return m_header;
}

int QGitDiffHunk::new_lines() const
{
    return m_new_lines;
}

int QGitDiffHunk::new_start() const
{
    return m_new_start;
}

int QGitDiffHunk::old_lines() const
{
    return m_old_lines;
}

int QGitDiffHunk::old_start() const
{
    return m_old_start;
}

QList<QGitDiffLine> QGitDiffHunk::lines() const
{
    return m_lines;
}
