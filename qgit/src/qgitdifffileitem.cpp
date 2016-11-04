#include "qgitdifffileitem.h"
#include <git2.h>

#include <QByteArray>


QGitDiffFileItem::QGitDiffFileItem()
{
}

QGitDiffFileItem::QGitDiffFileItem(const QString &path, const QByteArray &id, quint16 mode, quint32 flags, qint64 size)
    : m_path(path)
    , m_id(id)
    , m_mode(mode)
    , m_flags(flags)
    , m_size(size)
{
}

QGitDiffFileItem::QGitDiffFileItem(const QGitDiffFileItem &other)
    : m_path(other.m_path)
    , m_id(other.m_id)
    , m_mode(other.m_mode)
    , m_flags(other.m_flags)
    , m_size(other.m_size)
{
}

QGitDiffFileItem &QGitDiffFileItem::operator=(QGitDiffFileItem &&other)
{
    qSwap(m_path, other.m_path);
    qSwap(m_id, other.m_id);
    qSwap(m_mode, other.m_mode);
    qSwap(m_flags, other.m_flags);
    qSwap(m_size, other.m_size);

    return *this;
}

QGitDiffFileItem &QGitDiffFileItem::operator=(const QGitDiffFileItem &other)
{
    m_path = other.m_path;
    m_id = other.m_id;
    m_mode = other.m_mode;
    m_flags = other.m_flags;
    m_size = other.m_size;

    return *this;
}

QString QGitDiffFileItem::path() const
{
    return m_path;
}

QByteArray QGitDiffFileItem::id() const
{
    return m_id;
}

quint16 QGitDiffFileItem::mode() const
{
    return m_mode;
}

quint32 QGitDiffFileItem::flags() const
{
    return m_flags;
}

qint64 QGitDiffFileItem::size() const
{
    return m_size;
}

bool operator==(const QGitDiffFileItem &l, const git_diff_file &r)
{
    if (
            (l.path() == QString::fromUtf8(r.path))&&
            (l.id() == QByteArray((char *)r.id.id, sizeof(r.id)))&&
            (l.mode() == r.mode)&&
            (l.flags() == r.flags)&&
            (l.size() == r.size)
       )
    {
        return true;
    }

    return false;
}
