#pragma once

#include <QString>
#include <QFlags>

class QGitWorktree
{
public:
    enum class Flag {
        None     = 0,
        Main     = 1 << 0,
        Locked   = 1 << 1,
        Prunable = 1 << 2
    };
    Q_DECLARE_FLAGS(Flags, Flag)

    QGitWorktree() = default;
    QGitWorktree(const QString &name, const QString &path, const QString &branch,
                 Flags flags = Flag::None)
        : m_name(name), m_path(path), m_branch(branch), m_flags(flags)
    {}

    QString name() const { return m_name; }
    QString path() const { return m_path; }
    QString branch() const { return m_branch; }
    bool isMain() const { return m_flags.testFlag(Flag::Main); }
    bool isLocked() const { return m_flags.testFlag(Flag::Locked); }
    bool isPrunable() const { return m_flags.testFlag(Flag::Prunable); }

private:
    QString m_name;
    QString m_path;
    QString m_branch;
    Flags m_flags = Flag::None;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGitWorktree::Flags)
