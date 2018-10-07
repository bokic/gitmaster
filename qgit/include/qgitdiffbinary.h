#pragma once

#include "qgitdiffbinaryfile.h"
#include <git2.h>


class QGitDiffBinary
{
public:
    enum QGitDiffBinaryItem {OldFile, NewFile};

    QGitDiffBinary() = default;
    QGitDiffBinary(const git_diff_binary *binary);

    QGitDiffBinary(const QGitDiffBinary &other) = default;

    QGitDiffBinary &operator=(QGitDiffBinary &&other) noexcept;
    QGitDiffBinary &operator=(const QGitDiffBinary &other) = default;

    QGitDiffBinaryFile file(QGitDiffBinaryItem item) const;

private:
    QGitDiffBinaryFile m_oldFile;
    QGitDiffBinaryFile m_newFile;
};
