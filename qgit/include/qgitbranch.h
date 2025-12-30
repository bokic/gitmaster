#pragma once

#include <git2/types.h>
#include <stdint.h>

#include <QString>


class QGitBranch {
public:
    QGitBranch() = default;
    QGitBranch(const QString &name, int64_t time, git_branch_t type = GIT_BRANCH_ALL);
    QGitBranch(const QGitBranch &other) = default;
    QGitBranch(QGitBranch &&other) noexcept;
    virtual ~QGitBranch() = default;

    QGitBranch &operator=(QGitBranch &&other) noexcept;
    QGitBranch &operator=(const QGitBranch &other) = default;

    QString name() const;
    int64_t time() const;
    git_branch_t type() const;

private:
    QString m_name;
    int64_t m_time = 0;
    git_branch_t m_type = GIT_BRANCH_ALL;
};
