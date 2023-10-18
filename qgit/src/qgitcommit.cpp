#include "qgitcommit.h"


QGitCommit::QGitCommit(const QString &id, const QList<QGitCommitDiffParent> &parents, const QDateTime &time, const QGitSignature &author, const QGitSignature &commiter, const QString &message)
    : m_id(id)
    , m_parents(parents)
    , m_time(time)
    , m_author(author)
    , m_commiter(commiter)
    , m_message(message)
{
}

QGitCommit::QGitCommit(QGitCommit &&other) noexcept
{
    std::swap(m_id, other.m_id);
    std::swap(m_parents, other.m_parents);
    std::swap(m_time, other.m_time);
    std::swap(m_author, other.m_author);
    std::swap(m_commiter, other.m_commiter);
    std::swap(m_message, other.m_message);
}

QGitCommit &QGitCommit::operator=(QGitCommit &&other) noexcept
{
    std::swap(m_id, other.m_id);
    std::swap(m_parents, other.m_parents);
    std::swap(m_time, other.m_time);
    std::swap(m_author, other.m_author);
    std::swap(m_commiter, other.m_commiter);
    std::swap(m_message, other.m_message);

    return *this;
}

QString QGitCommit::id() const
{
    return m_id;
}

QList<QGitCommitDiffParent> QGitCommit::parents() const
{
    return m_parents;
}

QDateTime QGitCommit::time() const
{
    return m_time;
}

QGitSignature QGitCommit::author() const
{
    return m_author;
}

QGitSignature QGitCommit::commiter() const
{
    return m_commiter;
}

QString QGitCommit::message() const
{
    return m_message;
}
