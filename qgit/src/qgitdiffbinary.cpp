#include "qgitdiffbinary.h"


QGitDiffBinary::QGitDiffBinary(const git_diff_binary *binary)
    : m_oldFile(binary->old_file)
    , m_newFile(binary->new_file)
{
}

QGitDiffBinary &QGitDiffBinary::operator=(QGitDiffBinary &&other) noexcept
{
    std::swap(m_oldFile, other.m_oldFile);
    std::swap(m_newFile, other.m_newFile);

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
