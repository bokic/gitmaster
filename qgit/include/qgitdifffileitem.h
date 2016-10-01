#pragma once

#include <QByteArray>
#include <QString>


class QGitDiffFileItem
{
public:
    QGitDiffFileItem();
    QGitDiffFileItem(const QString &path, const QByteArray &id, quint16 mode, qint32 flags, qint64 size);
    QGitDiffFileItem(const QGitDiffFileItem &other);

    QGitDiffFileItem &operator=(QGitDiffFileItem &&other);
    QGitDiffFileItem &operator=(const QGitDiffFileItem &other);

    QString path() const;
    QByteArray id() const;
    quint16 mode() const;
    qint32 flags() const;
    qint64 size() const;

private:
    QString m_path;
    QByteArray m_id;
    quint16 m_mode = 0;
    qint32 m_flags = 0;
    qint64 m_size = 0;
};
