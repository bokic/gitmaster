#include <qgitsignature.h>


QGitSignature::QGitSignature(const QString &name, const QString &email, const QDateTime &when)
    : m_name(name)
    , m_email(email)
    , m_when(when)
{
}

QGitSignature &QGitSignature::operator=(QGitSignature &&other) noexcept
{
    std::swap(m_name, other.m_name);
    std::swap(m_email, other.m_email);
    std::swap(m_when, other.m_when);

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
