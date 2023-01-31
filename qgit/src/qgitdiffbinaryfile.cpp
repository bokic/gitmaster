#include "qgitdiffbinaryfile.h"


QGitDiffBinaryFile::QGitDiffBinaryFile()
    : m_type(GIT_DIFF_BINARY_NONE)
    , m_data()
    , m_inflatedLen(0)
{
}

QGitDiffBinaryFile::QGitDiffBinaryFile(const git_diff_binary_file &binary)
    : m_type(binary.type)
    , m_data(binary.data, static_cast<int>(binary.datalen))
    , m_inflatedLen(binary.inflatedlen)

{
}

QGitDiffBinaryFile &QGitDiffBinaryFile::operator=(QGitDiffBinaryFile &&other) noexcept
{
    std::swap(m_type, other.m_type);
    std::swap(m_data, other.m_data);
    std::swap(m_inflatedLen, other.m_inflatedLen);

    return *this;
}

git_diff_binary_t QGitDiffBinaryFile::type() const
{
    return m_type;
}

QByteArray QGitDiffBinaryFile::data() const
{
    return m_data;
}

size_t QGitDiffBinaryFile::inflatedLen() const
{
    return m_inflatedLen;
}
