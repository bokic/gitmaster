#pragma once

#include "git2.h"

#include <QDateTime>
#include <QString>


struct QGitBranch {
    QGitBranch(): name(), type(GIT_BRANCH_ALL) { }
    QGitBranch(const QString &_name, git_branch_t _type): name(_name), type(_type) {}

    QString name;
    git_branch_t type;

};

struct QGitSignature {
    QString name;
    QString email;
    QDateTime when;
};

struct QGitCommit {
    QString id;
    QList<QString> parents;
    QDateTime time;
    QGitSignature author;
    QGitSignature commiter;
    QString message;
};
