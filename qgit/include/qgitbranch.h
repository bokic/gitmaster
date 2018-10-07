#pragma once

#include "git2.h"

#include <QString>


class QGitBranch {
public:
    QGitBranch() = default;
    QGitBranch(const QString &name, git_branch_t type = GIT_BRANCH_ALL);
    QGitBranch(const QGitBranch &other) = default;

    QGitBranch &operator=(QGitBranch &&other) noexcept;
    QGitBranch &operator=(const QGitBranch &other) = default;

    QString name() const;
    git_branch_t type() const;

private:
    QString m_name;
    git_branch_t m_type = GIT_BRANCH_ALL;
};
