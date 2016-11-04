#include "qgitdiffhunk.h"
#include <git2.h>

#include <QString>
#include <QList>


QGitDiffHunk::QGitDiffHunk()
    : m_new_lines(-1)
    , m_new_start(-1)
    , m_old_lines(-1)
    , m_old_start(-1)
{
}

QGitDiffHunk::QGitDiffHunk(const git_diff_hunk *hunk)
    : m_header(QString::fromUtf8(hunk->header, static_cast<int>(hunk->header_len)))
    , m_new_lines(hunk->new_lines)
    , m_new_start(hunk->new_start)
    , m_old_lines(hunk->old_lines)
    , m_old_start(hunk->old_start)
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

void QGitDiffHunk::addLine(const QGitDiffLine &line)
{
    m_lines.append(line);
}

bool operator==(const QGitDiffHunk &l, const git_diff_hunk &r)
{

    if (
            (l.header() == QString::fromUtf8(r.header, static_cast<int>(r.header_len)))&&
            (l.new_lines() == r.new_lines)&&
            (l.new_start() == r.new_start)&&
            (l.old_lines() == r.old_lines)&&
            (l.old_start() == r.old_start)
       )
    {
        return true;
    }

    return false;
}
