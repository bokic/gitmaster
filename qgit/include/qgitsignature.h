#pragma once

#include <QDateTime>
#include <QString>


class QGitSignature {
public:
    QGitSignature();
    QGitSignature(const QString &name, const QString &email, const QDateTime &when);
    QGitSignature(const QGitSignature &other);

    QGitSignature &operator=(const QGitSignature &other);
    QGitSignature &operator=(QGitSignature &&other) noexcept;


    QString name() const;
    QString email() const;
    QDateTime when() const;

private:
    QString m_name;
    QString m_email;
    QDateTime m_when;
};
