#include "qgitdiffbinary.h"

QGitDiffBinary::QGitDiffBinary()
{
}

QGitDiffBinary::QGitDiffBinary(const git_diff_binary *binary)
    : m_oldFile(binary->old_file)
    , m_newFile(binary->new_file)
{
}

QGitDiffBinary::QGitDiffBinary(const QGitDiffBinary &other)
    : m_oldFile(other.m_oldFile)
    , m_newFile(other.m_newFile)
{
}

QGitDiffBinary &QGitDiffBinary::operator=(QGitDiffBinary &&other)
{
    qSwap(m_oldFile, other.m_oldFile);
    qSwap(m_newFile, other.m_newFile);

    return *this;
}

QGitDiffBinary &QGitDiffBinary::operator=(const QGitDiffBinary &other)
{
    m_oldFile = other.m_oldFile;
    m_newFile = other.m_newFile;

    return *this;
}

QGitDiffBinaryFile QGitDiffBinary::file(QGitDiffBinaryItem item) const
{
    switch(item)
    {
    case OldFile:
        return m_oldFile;
    case NewFile:
        return m_newFile;
    }

    return QGitDiffBinaryFile();
}
