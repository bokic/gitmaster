#include <qgitdiffline.h>


QGitDiffLine::QGitDiffLine()
{
}

QGitDiffLine::QGitDiffLine(const QString &content, int offset, int new_lineno, int num_lines, int old_lineno, char origin)
    : m_content(content)
    , m_offset(offset)
    , m_new_lineno(new_lineno)
    , m_num_lines(num_lines)
    , m_old_lineno(old_lineno)
    , m_origin(origin)
{
}

QGitDiffLine::QGitDiffLine(const QGitDiffLine &other)
    : m_content(other.m_content)
    , m_offset(other.m_offset)
    , m_new_lineno(other.m_new_lineno)
    , m_num_lines(other.m_num_lines)
    , m_old_lineno(other.m_old_lineno)
    , m_origin(other.m_origin)
{
}

QGitDiffLine &QGitDiffLine::operator=(QGitDiffLine &&other)
{
    qSwap(m_content, other.m_content);
    qSwap(m_offset, other.m_offset);
    qSwap(m_new_lineno, other.m_new_lineno);
    qSwap(m_num_lines, other.m_num_lines);
    qSwap(m_old_lineno, other.m_old_lineno);
    qSwap(m_origin, other.m_origin);

    return *this;
}

QGitDiffLine &QGitDiffLine::operator=(const QGitDiffLine &other)
{
    m_content = other.m_content;
    m_offset = other.m_offset;
    m_new_lineno = other.m_new_lineno;
    m_num_lines = other.m_num_lines;
    m_old_lineno = other.m_old_lineno;
    m_origin = other.m_origin;

    return *this;
}

QString QGitDiffLine::content() const
{
    return m_content;
}

int QGitDiffLine::offset() const
{
    return m_offset;
}

int QGitDiffLine::new_lineno() const
{
    return m_new_lineno;
}

int QGitDiffLine::num_lines() const
{
    return m_num_lines;
}

int QGitDiffLine::old_lineno() const
{
    return m_old_lineno;
}

char QGitDiffLine::origin() const
{
    return m_origin;
}
