#include "qgitcommitdiff.h"

QGitCommitDiff::QGitCommitDiff()
{

}

QGitCommitDiff::QGitCommitDiff(const QGitCommitDiff &other)
    : m_parents(other.m_parents)
{
}

QGitCommitDiff &QGitCommitDiff::operator=(QGitCommitDiff &&other)
{
    qSwap(m_parents, other.m_parents);

    return *this;
}

QGitCommitDiff &QGitCommitDiff::operator=(const QGitCommitDiff &other)
{
    m_parents = other.m_parents;

    return *this;
}

QList<QGitCommitDiffParent> QGitCommitDiff::parents() const
{
    return m_parents;
}
