#ifndef QGIT_H
#define QGIT_H

#include "git2.h"
#include "structs.h"

#include <QObject>
#include <QMap>
#include <QDir>

class QGit : public QObject
{
    Q_OBJECT
public:
    explicit QGit(QObject *parent = 0);
    virtual ~QGit();

    static QString getBranchNameFromPath(const QString &path);

public slots:
    void repositoryStatus(QDir path);
    void repositoryBranches(QDir path);

signals:
    void repositoryStatusReply(QDir path, QMap<git_status_t, int> items);
    void repositoryBranchesReply(QList<QGitBranch> branches);
    void error(QString qgit_function, QString git_function, int code);
};

#endif // QGIT_H
