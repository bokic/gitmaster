#include <qgitcommit.h>


QGitCommit::QGitCommit(const QString &id, const QList<QGitCommitDiffParent> &parents, const QDateTime &time, const QGitSignature &author, const QGitSignature &commiter, const QString &message, const QString &description, const QString &note)
    : m_id(id)
    , m_parents(parents)
    , m_time(time)
    , m_author(author)
    , m_commiter(commiter)
    , m_message(message)
    , m_description(description)
    , m_note(note)
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
    std::swap(m_description, other.m_description);
    std::swap(m_note, other.m_note);
}

QGitCommit &QGitCommit::operator=(QGitCommit &&other) noexcept
{
    std::swap(m_id, other.m_id);
    std::swap(m_parents, other.m_parents);
    std::swap(m_time, other.m_time);
    std::swap(m_author, other.m_author);
    std::swap(m_commiter, other.m_commiter);
    std::swap(m_message, other.m_message);
    std::swap(m_description, other.m_description);
    std::swap(m_note, other.m_note);

    return *this;
}

const QString &QGitCommit::id() const
{
    return m_id;
}

const QList<QGitCommitDiffParent> &QGitCommit::parents() const
{
    return m_parents;
}

const QDateTime &QGitCommit::time() const
{
    return m_time;
}

const QGitSignature &QGitCommit::author() const
{
    return m_author;
}

const QGitSignature &QGitCommit::commiter() const
{
    return m_commiter;
}

const QString &QGitCommit::message() const
{
    return m_message;
}

const QString &QGitCommit::description() const
{
    return m_description;
}

const QString &QGitCommit::note() const
{
    return m_note;
}
