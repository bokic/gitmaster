#include "qgiterror.h"

QGitError::QGitError()
{
}

QGitError::QGitError(const QString functionName, int errorCode)
    : m_functionName(functionName)
    , m_errorCode(errorCode)
{
}

QGitError::QGitError(const QGitError &other)
    : m_functionName(other.m_functionName)
    , m_errorCode(other.m_errorCode)
{
}

QGitError &QGitError::operator=(QGitError &&other)
{
    qSwap(m_functionName, other.m_functionName);
    qSwap(m_errorCode, other.m_errorCode);

    return *this;
}

QGitError &QGitError::operator=(const QGitError &other)
{
    m_functionName = other.m_functionName;
    m_errorCode = other.m_errorCode;

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
