#include <qgiterror.h>
#include <git2.h>


QGitError::QGitError()
{
    const git_error *err = git_error_last();
    if (err) {
        m_errorString = QString::fromUtf8(err->message);
    } else {
        m_errorString = QStringLiteral("Unknown Git error");
    }
}

QGitError::QGitError(const QString &functionName, int errorCode)
    : m_functionName(functionName)
    , m_errorCode(errorCode)
{
    const git_error *err = git_error_last();
    if (err) {
        m_errorString = QString::fromUtf8(err->message);
    } else {
        m_errorString = QStringLiteral("Unknown Git error");
    }
}

QGitError::QGitError(QGitError &&other) noexcept
{
    std::swap(m_functionName, other.m_functionName);
    std::swap(m_errorString, other.m_errorString);
    std::swap(m_errorCode, other.m_errorCode);
}

QGitError &QGitError::operator=(QGitError &&other) noexcept
{
    std::swap(m_functionName, other.m_functionName);
    std::swap(m_errorString, other.m_errorString);
    std::swap(m_errorCode, other.m_errorCode);

    return *this;
}

QString QGitError::functionName() const
{
    return m_functionName;
}

QString QGitError::errorString() const
{
    return m_errorString;
}

int QGitError::errorCode() const
{
    return m_errorCode;
}
