#include "qgitbranch.h"
#include "git2.h"

#include <QString>


QGitBranch::QGitBranch()
{
}

QGitBranch::QGitBranch(const QString &name, git_branch_t type)
    : m_name(name)
    , m_type(type)
{
}

QGitBranch::QGitBranch(const QGitBranch &other)
    : m_name(other.m_name)
    , m_type(other.m_type)
{
}

QGitBranch &QGitBranch::operator=(QGitBranch &&other)
{
    qSwap(m_name, other.m_name);
    qSwap(m_type, other.m_type);

    return *this;
}

QGitBranch &QGitBranch::operator=(const QGitBranch &other)
{
    m_name = other.m_name;
    m_type = other.m_type;

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
