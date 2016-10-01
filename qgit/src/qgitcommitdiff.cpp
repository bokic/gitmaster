#include "qgitcommitdiff.h"

QGitCommitDiff::QGitCommitDiff()
{

}

QGitCommitDiff::QGitCommitDiff(QString parentHash, QList<QGitDiffFile> files)
    : m_parentHash(parentHash)
    , m_files(files)
{
}

QGitCommitDiff::QGitCommitDiff(const QGitCommitDiff &other)
    : m_parentHash(other.m_parentHash)
    , m_files(other.m_files)
{
}

QGitCommitDiff &QGitCommitDiff::operator=(QGitCommitDiff &&other)
{
    qSwap(m_parentHash, other.m_parentHash);
    qSwap(m_files, other.m_files);

    return *this;
}

QGitCommitDiff &QGitCommitDiff::operator=(const QGitCommitDiff &other)
{
    m_parentHash = other.m_parentHash;
    m_files = other.m_files;

    return *this;
}

QString QGitCommitDiff::parentHash() const
{
    return m_parentHash;
}

QList<QGitDiffFile> QGitCommitDiff::files() const
{
    return m_files;
}
