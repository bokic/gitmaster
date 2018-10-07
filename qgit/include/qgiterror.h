#pragma once

#include <QString>


class QGitError
{
public:
    QGitError() = default;
    QGitError(const QString &functionName, int errorCode);

    QGitError(const QGitError &other) = default;

    QGitError &operator=(QGitError &&other) noexcept;
    QGitError &operator=(const QGitError &other) = default;

    QString functionName() const;
    int errorCode() const;

private:
    QString m_functionName;
    int m_errorCode = 0;
};
