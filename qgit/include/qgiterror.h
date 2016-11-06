#pragma once

#include <QString>


class QGitError
{
public:
    QGitError();
    QGitError(const QString functionName, int errorCode);

    QGitError(const QGitError &other);

    QGitError &operator=(QGitError &&other);
    QGitError &operator=(const QGitError &other);

    QString functionName() const;
    int errorCode() const;

private:
    QString m_functionName;
    int m_errorCode = 0;
};
