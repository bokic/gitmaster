#pragma once

#include <QDateTime>
#include <QString>


class QGitSignature {
public:
    QGitSignature() = default;
    QGitSignature(const QString &name, const QString &email, const QDateTime &when);
    QGitSignature(const QGitSignature &other) = default;
    QGitSignature(QGitSignature &&other) noexcept;

    QGitSignature &operator=(QGitSignature &&other) noexcept;
    QGitSignature &operator=(const QGitSignature &other) = default;


    QString name() const;
    QString email() const;
    QDateTime when() const;

private:
    QString m_name;
    QString m_email;
    QDateTime m_when;
};
