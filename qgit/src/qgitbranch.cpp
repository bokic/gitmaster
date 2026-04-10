#include <qgitbranch.h>

#include <QString>


QGitBranch::QGitBranch(const QString &name, const QString &hash, int64_t time, git_branch_t type)
    : m_name(name)
    , m_hash(hash)
    , m_time(time)
    , m_type(type)
{
}

QGitBranch::QGitBranch(QGitBranch &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_hash, other.m_hash);
    std::swap(m_time, other.m_time);
    std::swap(m_type, other.m_type);
}

QGitBranch &QGitBranch::operator=(QGitBranch &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_hash, other.m_hash);
    std::swap(m_time, other.m_time);
    std::swap(m_type, other.m_type);

    return *this;
}

QString QGitBranch::name() const
{
    return m_name;
}

QString QGitBranch::hash() const
{
    return m_hash;
}

int64_t QGitBranch::time() const
{
    return m_time;
}

git_branch_t QGitBranch::type() const
{
    return m_type;
}
