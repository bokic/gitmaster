#include <qgitbranch.h>

#include <QString>


QGitBranch::QGitBranch(const QString &name, git_branch_t type)
    : m_name(name)
    , m_type(type)
{
}

QGitBranch::QGitBranch(QGitBranch &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_type, other.m_type);
}

QGitBranch &QGitBranch::operator=(QGitBranch &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_type, other.m_type);

    return *this;
}

QString QGitBranch::name() const
{
    return m_name;
}

git_branch_t QGitBranch::type() const
{
    return m_type;
}
