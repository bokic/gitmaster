#include "qgitdifffile.h"
#include "qgitdifffileitem.h"
#include "qgitdiffhunk.h"

#include <QList>


QGitDiffFile::QGitDiffFile(const git_diff_delta *file)
    : m_new_file(file->new_file)
    , m_old_file(file->old_file)
    , m_flags(file->flags)
    , m_nfiles(file->nfiles)
    , m_simularity(file->similarity)
    , m_status(file->status)
{
}

QGitDiffFile::QGitDiffFile(const QGitDiffFile &other)
    : m_new_file(other.m_new_file)
    , m_old_file(other.m_old_file)
    , m_flags(other.m_flags)
    , m_nfiles(other.m_nfiles)
    , m_simularity(other.m_simularity)
    , m_status(other.m_status)
    , m_hunks(other.m_hunks)
{
}

QGitDiffFile::QGitDiffFile(QGitDiffFile &&other) noexcept
{
    std::swap(m_new_file, other.m_new_file);
    std::swap(m_old_file, other.m_old_file);
    std::swap(m_flags, other.m_flags);
    std::swap(m_nfiles, other.m_nfiles);
    std::swap(m_simularity, other.m_simularity);
    std::swap(m_status, other.m_status);
    std::swap(m_hunks, other.m_hunks);
}

QGitDiffFile &QGitDiffFile::operator=(QGitDiffFile &&other) noexcept
{
    std::swap(m_new_file, other.m_new_file);
    std::swap(m_old_file, other.m_old_file);
    std::swap(m_flags, other.m_flags);
    std::swap(m_nfiles, other.m_nfiles);
    std::swap(m_simularity, other.m_simularity);
    std::swap(m_status, other.m_status);
    std::swap(m_hunks, other.m_hunks);

    return *this;
}

QGitDiffFile &QGitDiffFile::operator=(const QGitDiffFile &other)
{
    m_new_file = other.m_new_file;
    m_old_file = other.m_old_file;
    m_flags = other.m_flags;
    m_nfiles = other.m_nfiles;
    m_simularity = other.m_simularity;
    m_status = other.m_status;
    m_hunks = other.m_hunks;

    return *this;
}

bool QGitDiffFile::operator==(const git_diff_delta *other)
{
    if (
            (other)&&
            (m_flags == other->flags)&&
            (m_new_file == other->new_file)&&
            (m_nfiles == other->nfiles)&&
            (m_old_file == other->old_file)&&
            (m_simularity == other->similarity)&&
            (m_status == other->status)
       )
    {
        return true;
    }

    return false;
}

bool QGitDiffFile::operator==(const QGitDiffFile &other)
{
    if (
            (m_flags == other.m_flags)&&
            (m_new_file == other.m_new_file)&&
            (m_nfiles == other.m_nfiles)&&
            (m_old_file == other.m_old_file)&&
            (m_simularity == other.m_simularity)&&
            (m_status == other.m_status)
       )
    {
        return true;
    }

    return false;
}

QGitDiffFileItem QGitDiffFile::new_file() const
{
    return m_new_file;
}

QGitDiffFileItem QGitDiffFile::old_file() const
{
    return m_old_file;
}

uint32_t QGitDiffFile::flags() const
{
    return m_flags;
}

int QGitDiffFile::nfiles() const
{
    return m_nfiles;
}

int QGitDiffFile::simularity() const
{
    return m_simularity;
}

int QGitDiffFile::status() const
{
    return m_status;
}

QList<QGitDiffHunk> QGitDiffFile::hunks() const
{
    return m_hunks;
}

QList<QGitDiffBinary> QGitDiffFile::binaries() const
{
    return m_binary;
}

void QGitDiffFile::addHunk(const QGitDiffHunk &hunk)
{
    m_hunks.append(hunk);
}

void QGitDiffFile::addBinary(const QGitDiffBinary &binary)
{
    m_binary.append(binary);
}
