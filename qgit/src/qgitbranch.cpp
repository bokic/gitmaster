#include <qgitbranch.h>

#include <QString>


QGitBranch::QGitBranch(const QString &name, const QString &hash, int64_t time, git_branch_t type, int ahead, int behind)
    : m_name(name)
    , m_hash(hash)
    , m_time(time)
    , m_type(type)
    , m_ahead(ahead)
    , m_behind(behind)
{
}

QGitBranch::QGitBranch(QGitBranch &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_hash, other.m_hash);
    std::swap(m_time, other.m_time);
    std::swap(m_type, other.m_type);
    std::swap(m_ahead, other.m_ahead);
    std::swap(m_behind, other.m_behind);
}

QGitBranch &QGitBranch::operator=(QGitBranch &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_hash, other.m_hash);
    std::swap(m_time, other.m_time);
    std::swap(m_type, other.m_type);
    std::swap(m_ahead, other.m_ahead);
    std::swap(m_behind, other.m_behind);

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

int QGitBranch::ahead() const
{
    return m_ahead;
}

int QGitBranch::behind() const
{
    return m_behind;
}
