#include <qgitdifffile.h>
#include <qgitdifffileitem.h>
#include <qgitdiffhunk.h>

#include <QList>


QGitDiffFile::QGitDiffFile(const git_diff_delta *file)
    : m_new_file(file ? file->new_file : git_diff_file{})
    , m_old_file(file ? file->old_file : git_diff_file{})
    , m_flags(file ? file->flags : 0)
    , m_nfiles(file ? file->nfiles : 0)
    , m_simularity(file ? file->similarity : 0)
    , m_status(file ? file->status : GIT_DELTA_UNMODIFIED)
{
}
bool QGitDiffFile::operator==(const git_diff_delta *other) const
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

bool QGitDiffFile::operator==(const QGitDiffFile &other) const
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

const QGitDiffFileItem &QGitDiffFile::new_file() const
{
    return m_new_file;
}

const QGitDiffFileItem &QGitDiffFile::old_file() const
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

const QList<QGitDiffHunk> &QGitDiffFile::hunks() const
{
    return m_hunks;
}

const QList<QGitDiffBinary> &QGitDiffFile::binaries() const
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
