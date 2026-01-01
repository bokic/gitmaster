#include <qgiterror.h>
#include <git2.h>


QGitError::QGitError()
    : m_errorString(git_error_last()->message)
{
}

QGitError::QGitError(const QString &functionName, int errorCode)
    : m_functionName(functionName)
    , m_errorString(git_error_last()->message)
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
