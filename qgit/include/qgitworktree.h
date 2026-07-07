#pragma once

#include <QString>

class QGitWorktree
{
public:
    QGitWorktree() = default;
    QGitWorktree(const QString &name, const QString &path, const QString &branch,
                 bool isMain = false, bool isLocked = false, bool isPrunable = false)
        : m_name(name), m_path(path), m_branch(branch),
          m_isMain(isMain), m_isLocked(isLocked), m_isPrunable(isPrunable)
    {}

    QString name() const { return m_name; }
    QString path() const { return m_path; }
    QString branch() const { return m_branch; }
    bool isMain() const { return m_isMain; }
    bool isLocked() const { return m_isLocked; }
    bool isPrunable() const { return m_isPrunable; }

private:
    QString m_name;
    QString m_path;
    QString m_branch;
    bool m_isMain    = false;
    bool m_isLocked  = false;
    bool m_isPrunable = false;
};
