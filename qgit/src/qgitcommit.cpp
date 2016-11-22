#include "qgitcommit.h"


QGitCommit::QGitCommit()
{
}

QGitCommit::QGitCommit(const QString &id, const QList<QGitCommitDiffParent> &parents, const QDateTime &time, const QGitSignature &author, const QGitSignature &commiter, const QString &message)
    : m_id(id)
    , m_parents(parents)
    , m_time(time)
    , m_author(author)
    , m_commiter(commiter)
    , m_message(message)
{
}

QGitCommit::QGitCommit(const QGitCommit &other)
    : m_id(other.m_id)
    , m_parents(other.m_parents)
    , m_time(other.m_time)
    , m_author(other.m_author)
    , m_commiter(other.m_commiter)
    , m_message(other.m_message)
{
}

QGitCommit &QGitCommit::operator=(QGitCommit &&other)
{
    qSwap(m_id, other.m_id);
    qSwap(m_parents, other.m_parents);
    qSwap(m_time, other.m_time);
    qSwap(m_author, other.m_author);
    qSwap(m_commiter, other.m_commiter);
    qSwap(m_message, other.m_message);

    return *this;
}

QGitCommit &QGitCommit::operator=(const QGitCommit &other)
{
    m_id = other.m_id;
    m_parents = other.m_parents;
    m_time = other.m_time;
    m_author = other.m_author;
    m_commiter = other.m_commiter;
    m_message = other.m_message;

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
