#pragma once

#include <git2.h>
#include <stdint.h>

#include <QString>


class QGitBranch {
public:
    QGitBranch() = default;
    QGitBranch(const QString &name, const QString &hash, int64_t time, git_branch_t type = GIT_BRANCH_ALL, int ahead = 0, int behind = 0);
    QGitBranch(const QGitBranch &other) = default;
    QGitBranch(QGitBranch &&other) noexcept;
    virtual ~QGitBranch() = default;

    QGitBranch &operator=(QGitBranch &&other) noexcept;
    QGitBranch &operator=(const QGitBranch &other) = default;

    QString name() const;
    QString hash() const;
    int64_t time() const;
    git_branch_t type() const;
    int ahead() const;
    int behind() const;

private:
    QString m_name;
    QString m_hash;
    int64_t m_time = 0;
    git_branch_t m_type = GIT_BRANCH_ALL;
    int m_ahead = 0;
    int m_behind = 0;
};
