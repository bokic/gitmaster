#pragma once

#include <QString>


class QGitError
{
public:
    QGitError();
    QGitError(const QString &functionName, int errorCode);
    QGitError(const QGitError &other) = default;
    QGitError(QGitError &&other) noexcept;
    virtual ~QGitError() = default;

    QGitError &operator=(QGitError &&other) noexcept;
    QGitError &operator=(const QGitError &other) = default;

    QString functionName() const;
    QString errorString() const;
    int errorCode() const;

private:
    QString m_functionName;
    QString m_errorString;
    int m_errorCode = 0;
};
