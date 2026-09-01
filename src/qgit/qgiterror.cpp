#include <qgiterror.h>
#include <git2.h>


QGitError::QGitError()
    : m_functionName()
    , m_errorString()
    , m_errorCode(0)
{
}

QGitError::QGitError(const QString &functionName, int errorCode)
    : m_functionName(functionName)
    , m_errorCode(errorCode)
{
    const git_error *err = git_error_last();
    if (err && err->message) {
        m_errorString = QString::fromUtf8(err->message);
    } else {
        m_errorString = QString("Git error %1: %2").arg(errorCode).arg(functionName);
    }
}

QGitError::QGitError(const QString &functionName, int errorCode, const QString &customMessage)
    : m_functionName(functionName)
    , m_errorString(customMessage.isEmpty() ? QString("Git error %1: %2").arg(errorCode).arg(functionName) : customMessage)
    , m_errorCode(errorCode)
{
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
