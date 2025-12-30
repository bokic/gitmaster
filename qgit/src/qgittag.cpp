#include <qgittag.h>

#include <QString>


QGitTag::QGitTag(const QString &name, int64_t time)
    : m_name(name)
    , m_time(time)
{
}

QGitTag::QGitTag(QGitTag &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_time, other.m_time);
}

QGitTag &QGitTag::operator=(QGitTag &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_time, other.m_time);

    return *this;
}

QString QGitTag::name() const
{
    return m_name;
}

int64_t QGitTag::time() const
{
    return m_time;
}
