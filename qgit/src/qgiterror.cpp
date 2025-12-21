#include <qgiterror.h>


QGitError::QGitError(const QString &functionName, int errorCode)
    : m_functionName(functionName)
    , m_errorCode(errorCode)
{
}

QGitError::QGitError(QGitError &&other) noexcept
{
    std::swap(m_functionName, other.m_functionName);
    std::swap(m_errorCode, other.m_errorCode);
}

QGitError &QGitError::operator=(QGitError &&other) noexcept
{
    std::swap(m_functionName, other.m_functionName);
    std::swap(m_errorCode, other.m_errorCode);

    return *this;
}

QString QGitError::functionName() const
{
    return m_functionName;
}

int QGitError::errorCode() const
{
    return m_errorCode;
}
