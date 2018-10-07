#pragma once

#include <QByteArray>
#include <QString>
#include <git2.h>


class QGitDiffFileItem
{
public:
    QGitDiffFileItem();
    QGitDiffFileItem(const git_diff_file &file);
    QGitDiffFileItem(const QGitDiffFileItem &other);

    QGitDiffFileItem &operator=(const QGitDiffFileItem &other);
    QGitDiffFileItem &operator=(QGitDiffFileItem &&other) noexcept;

    QString path() const;
    QByteArray id() const;
    quint16 mode() const;
    quint32 flags() const;
    qint64 size() const;

private:
    QString m_path;
    QByteArray m_id;
    quint16 m_mode = 0;
    quint32 m_flags = 0;
    qint64 m_size = 0;
};

bool operator==(const QGitDiffFileItem &l, const git_diff_file &r);
