#include <qgitdifffileitem.h>

#include <git2.h>
#include <cstring>

#include <QByteArray>


QGitDiffFileItem::QGitDiffFileItem(const git_diff_file &file)
    : m_path(file.path)
    , m_id(reinterpret_cast<const char *>(file.id.id), sizeof(file.id.id))
    , m_mode(file.mode)
    , m_flags(file.flags)
    , m_size(file.size)
{
}

QGitDiffFileItem::QGitDiffFileItem(QGitDiffFileItem &&other) noexcept
{
    std::swap(m_path, other.m_path);
    std::swap(m_id, other.m_id);
    std::swap(m_mode, other.m_mode);
    std::swap(m_flags, other.m_flags);
    std::swap(m_size, other.m_size);
}

QGitDiffFileItem &QGitDiffFileItem::operator=(QGitDiffFileItem &&other) noexcept
{
    std::swap(m_path, other.m_path);
    std::swap(m_id, other.m_id);
    std::swap(m_mode, other.m_mode);
    std::swap(m_flags, other.m_flags);
    std::swap(m_size, other.m_size);

    return *this;
}

bool QGitDiffFileItem::operator==(const QGitDiffFileItem &other) const
{
    if (
            (m_path == other.m_path)&&
            (m_id == other.m_id)&&
            (m_mode == other.m_mode)&&
            (m_flags == other.m_flags)&&
            (m_size == other.m_size)
       )
    {
        return true;
    }

    return false;
}

bool QGitDiffFileItem::operator==(const git_diff_file &other) const
{
    if (
            (m_path == other.path)&&
            (m_id.size() == static_cast<int>(sizeof(other.id.id)))&&
            (memcmp(m_id.constData(), other.id.id, sizeof(other.id.id)) == 0)&&
            (m_mode == other.mode)&&
            (m_flags == other.flags)&&
            (m_size == other.size)
       )
    {
        return true;
    }

    return false;
}

const QString &QGitDiffFileItem::path() const
{
    return m_path;
}

const QByteArray &QGitDiffFileItem::id() const
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

quint64 QGitDiffFileItem::size() const
{
    return m_size;
}
