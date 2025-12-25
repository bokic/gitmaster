#pragma once

#include <QByteArray>
#include <QString>
#include <git2.h>


class QGitDiffFileItem
{
public:
    QGitDiffFileItem() = default;
    QGitDiffFileItem(const git_diff_file &file);
    QGitDiffFileItem(const QGitDiffFileItem &other) = default;
    QGitDiffFileItem(QGitDiffFileItem &&other) noexcept;
    virtual ~QGitDiffFileItem() = default;

    QGitDiffFileItem &operator=(QGitDiffFileItem &&other) noexcept;
    QGitDiffFileItem &operator=(const QGitDiffFileItem &other) = default;

    bool operator==(const QGitDiffFileItem &other);
    bool operator==(const git_diff_file &other);

    QString path() const;
    QByteArray id() const;
    quint16 mode() const;
    quint32 flags() const;
    quint64 size() const;

private:
    QString m_path;
    QByteArray m_id;
    quint16 m_mode = 0;
    quint32 m_flags = 0;
    quint64 m_size = 0;
};

bool operator==(const QGitDiffFileItem &l, const git_diff_file &r);
