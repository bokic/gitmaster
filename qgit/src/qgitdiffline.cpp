#include <qgitdiffline.h>


QGitDiffLine::QGitDiffLine(const QByteArray &content, git_off_t offset, int new_lineno, int num_lines, int old_lineno, char origin)
    : m_content(content)
    , m_offset(offset)
    , m_new_lineno(new_lineno)
    , m_num_lines(num_lines)
    , m_old_lineno(old_lineno)
    , m_origin(origin)
{
}

QGitDiffLine::QGitDiffLine(const git_diff_line *line)
    : m_content(QByteArray(line->content, static_cast<int>(line->content_len)))
    , m_offset(line->content_offset)
    , m_new_lineno(line->new_lineno)
    , m_num_lines(line->num_lines)
    , m_old_lineno(line->old_lineno)
    , m_origin(line->origin)
{
}

QGitDiffLine::QGitDiffLine(QGitDiffLine &&other) noexcept
{
    std::swap(m_content, other.m_content);
    std::swap(m_offset, other.m_offset);
    std::swap(m_new_lineno, other.m_new_lineno);
    std::swap(m_num_lines, other.m_num_lines);
    std::swap(m_old_lineno, other.m_old_lineno);
    std::swap(m_origin, other.m_origin);
}

QGitDiffLine &QGitDiffLine::operator=(QGitDiffLine &&other) noexcept
{
    std::swap(m_content, other.m_content);
    std::swap(m_offset, other.m_offset);
    std::swap(m_new_lineno, other.m_new_lineno);
    std::swap(m_num_lines, other.m_num_lines);
    std::swap(m_old_lineno, other.m_old_lineno);
    std::swap(m_origin, other.m_origin);

    return *this;
}

QByteArray QGitDiffLine::content() const
{
    return m_content;
}

git_off_t QGitDiffLine::offset() const
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
