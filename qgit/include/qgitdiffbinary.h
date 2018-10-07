#pragma once

#include "qgitdiffbinaryfile.h"
#include <git2.h>


class QGitDiffBinary
{
public:
    enum QGitDiffBinaryItem {OldFile, NewFile};

    QGitDiffBinary();
    QGitDiffBinary(const git_diff_binary *binary);

    QGitDiffBinary(const QGitDiffBinary &other);

    QGitDiffBinary &operator=(const QGitDiffBinary &other);
    QGitDiffBinary &operator=(QGitDiffBinary &&other) noexcept;

    QGitDiffBinaryFile file(QGitDiffBinaryItem item) const;

private:
    QGitDiffBinaryFile m_oldFile;
    QGitDiffBinaryFile m_newFile;
};
