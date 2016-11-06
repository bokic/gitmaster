#include "qgitcommitdiffparent.h"
#include "qgitdiffbinary.h"


QGitCommitDiffParent::QGitCommitDiffParent(QByteArray commitHash)
    : m_commitHash(commitHash)
{
}

QGitCommitDiffParent::QGitCommitDiffParent(const QGitCommitDiffParent &other)
    : m_commitHash(other.m_commitHash)
    , m_files(other.m_files)
{
}

QGitCommitDiffParent &QGitCommitDiffParent::operator=(QGitCommitDiffParent &&other)
{
    qSwap(m_commitHash, other.m_commitHash);
    qSwap(m_files, other.m_files);

    return *this;
}

QGitCommitDiffParent &QGitCommitDiffParent::operator=(const QGitCommitDiffParent &other)
{
    m_commitHash = other.m_commitHash;
    m_files = other.m_files;

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

void QGitCommitDiffParent::addBinary(const git_diff_delta *delta, const git_diff_binary *binary)
{
    for(QGitDiffFile &file: m_files)
    {
        if (file == delta)
        {
            QGitDiffBinary item(binary);

            file.addBinary(item);
        }
    }
}

void QGitCommitDiffParent::addHunk(const git_diff_delta *delta, const git_diff_hunk *hunk)
{
    for(QGitDiffFile &file: m_files)
    {
        if (file == delta)
        {
            QGitDiffHunk item(hunk);

            file.addHunk(item);
        }
    }
}

void QGitCommitDiffParent::addLine(const git_diff_delta *delta, const git_diff_hunk *hunk, const git_diff_line *line)
{
    for(QGitDiffFile &file: m_files)
    {
        if (file == delta)
        {
            for(QGitDiffHunk &item: file.m_hunks)
            {
                if (item == *hunk)
                {
                    QGitDiffLine _line(line);

                    item.addLine(_line);
                }
            }
        }
    }
}

bool QGitCommitDiffParent::hasFile(const git_diff_delta *delta)
{
    for(auto item: m_files)
    {
        if (item == delta)
        {
            return true;
        }
    }

    return false;
}
