#include "qgitdifffile.h"
#include "qgitdifffileitem.h"
#include "qgitdiffhunk.h"

#include <QList>


QGitDiffFile::QGitDiffFile()
{
}

QGitDiffFile::QGitDiffFile(const QGitDiffFileItem &new_file, const QGitDiffFileItem &old_file, int flags, int nfiles, int simularity, int status, const QList<QGitDiffHunk> &hunks)
    : m_new_file(new_file)
    , m_old_file(old_file)
    , m_flags(flags)
    , m_nfiles(nfiles)
    , m_simularity(simularity)
    , m_status(status)
    , m_hunks(hunks)
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

QGitDiffFile &QGitDiffFile::operator=(QGitDiffFile &&other)
{
    qSwap(m_new_file, other.m_new_file);
    qSwap(m_old_file, other.m_old_file);
    qSwap(m_flags, other.m_flags);
    qSwap(m_nfiles, other.m_nfiles);
    qSwap(m_simularity, other.m_simularity);
    qSwap(m_status, other.m_status);
    qSwap(m_hunks, other.m_hunks);

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

QGitDiffFileItem QGitDiffFile::new_file() const
{
    return m_new_file;
}

QGitDiffFileItem QGitDiffFile::old_file() const
{
    return m_old_file;
}

int QGitDiffFile::flags() const
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
