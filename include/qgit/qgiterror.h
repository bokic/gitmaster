#pragma once

#include <QString>


class QGitError
{
public:
    QGitError();
    QGitError(const QString &functionName, int errorCode);
    QGitError(const QString &functionName, int errorCode, const QString &customMessage);
    QGitError(const QGitError &other) = default;
    QGitError(QGitError &&other) noexcept;
    virtual ~QGitError() = default;

    QGitError &operator=(QGitError &&other) noexcept;
    QGitError &operator=(const QGitError &other) = default;

    QString functionName() const;
    QString errorString() const;
    int errorCode() const;
    bool hasError() const { return m_errorCode != 0; }
    bool isOk() const { return m_errorCode == 0; }
    explicit operator bool() const { return hasError(); }

private:
    QString m_functionName;
    QString m_errorString;
    int m_errorCode = 0;
};
