#pragma once

#include <QByteArray>
#include <QString>
#include <exception>


class QGitError : public std::exception
{
public:
    QGitError();
    explicit QGitError(const QString &functionName, int errorCode = -1);
    QGitError(const QString &functionName, int errorCode, const QString &customMessage);
    QGitError(const QGitError &other) = default;
    QGitError(QGitError &&other) noexcept;
    ~QGitError() override = default;

    QGitError &operator=(QGitError &&other) noexcept;
    QGitError &operator=(const QGitError &other) = default;

    const char *what() const noexcept override;
    QString functionName() const;
    QString errorString() const;
    int errorCode() const;
    bool hasError() const { return m_errorCode != 0 || !m_errorString.isEmpty(); }
    bool isOk() const { return !hasError(); }
    explicit operator bool() const { return hasError(); }

private:
    QString m_functionName;
    QString m_errorString;
    mutable QByteArray m_whatCache;
    int m_errorCode = 0;
};
