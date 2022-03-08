#include "qgitcommitdiffparent.h"
#include "qgitdiffbinary.h"


QGitCommitDiffParent::QGitCommitDiffParent(const QByteArray &commitHash)
    : m_commitHash(commitHash)
{
}

QGitCommitDiffParent &QGitCommitDiffParent::operator=(QGitCommitDiffParent &&other) noexcept
{
    std::swap(m_commitHash, other.m_commitHash);
    std::swap(m_files, other.m_files);

    return *this;
}

QByteArray QGitCommitDiffParent::commitHash() const
{
    return m_commitHash;
}

QList<QGitDiffFile> QGitCommitDiffParent::files() const
{
    return m_files;
}

void QGitCommitDiffParent::addFile(const git_diff_delta *delta)
{
    QGitDiffFile file(delta);

    m_files.append(file);
}
