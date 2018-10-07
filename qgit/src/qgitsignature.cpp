#include <qgitsignature.h>


QGitSignature::QGitSignature(const QString &name, const QString &email, const QDateTime &when)
    : m_name(name)
    , m_email(email)
    , m_when(when)
{
}

QGitSignature &QGitSignature::operator=(QGitSignature &&other) noexcept
{
    qSwap(m_name, other.m_name);
    qSwap(m_email, other.m_email);
    qSwap(m_when, other.m_when);

    return *this;
}

QString QGitSignature::name() const
{
    return m_name;
}

QString QGitSignature::email() const
{
    return m_email;
}

QDateTime QGitSignature::when() const
{
    return m_when;
}
