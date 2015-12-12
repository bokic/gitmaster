#ifndef STRUCTS_H
#define STRUCTS_H

#include "git2.h"

#include <QString>


struct QGitBranch {
    QGitBranch(): name(), type(GIT_BRANCH_ALL) { }
    QGitBranch(const QString &_name, git_branch_t _type): name(_name), type(_type) {}

    QString name;
    git_branch_t type;

};

#endif // STRUCTS_H

