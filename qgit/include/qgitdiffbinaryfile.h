#pragma once

#include <git2.h>

#include <QByteArray>


class QGitDiffBinaryFile
{
public:
    QGitDiffBinaryFile();
    QGitDiffBinaryFile(const git_diff_binary_file &binary);
    QGitDiffBinaryFile(const QGitDiffBinaryFile &other) = default;
    QGitDiffBinaryFile(QGitDiffBinaryFile &&other) noexcept;
    virtual ~QGitDiffBinaryFile() = default;

    QGitDiffBinaryFile &operator=(QGitDiffBinaryFile &&other) noexcept;
    QGitDiffBinaryFile &operator=(const QGitDiffBinaryFile &other) = default;

    git_diff_binary_t type() const;
    QByteArray data() const;
    size_t inflatedLen() const;

private:
    git_diff_binary_t m_type;
    QByteArray m_data;
    size_t m_inflatedLen;
};
