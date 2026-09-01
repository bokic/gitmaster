#pragma once

#include <QString>
#include <QMetaType>
#include <QList>

enum class QGitRebaseAction {
    Pick = 0,
    Reword,
    Squash,
    Fixup,
    Drop
};

struct QGitRebaseTodoItem {
    QGitRebaseAction action = QGitRebaseAction::Pick;
    QString commitId;
    QString shortHash;
    QString summary;
    QString message;     // Full commit message (can be edited for reword / squash)
    QString author;
    QString timeStr;
};

Q_DECLARE_METATYPE(QGitRebaseAction)
Q_DECLARE_METATYPE(QGitRebaseTodoItem)
Q_DECLARE_METATYPE(QList<QGitRebaseTodoItem>)
