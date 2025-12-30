#pragma once

#include <QString>

#include <stdint.h>


class QGitTag {
public:
    QGitTag() = default;
    QGitTag(const QString &name, int64_t time);
    QGitTag(const QGitTag &other) = default;
    QGitTag(QGitTag &&other) noexcept;
    virtual ~QGitTag() = default;

    QGitTag &operator=(QGitTag &&other) noexcept;
    QGitTag &operator=(const QGitTag &other) = default;

    QString name() const;
    int64_t time() const;

private:
    QString m_name;
    int64_t m_time = 0;
};
