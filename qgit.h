#ifndef QGIT_H
#define QGIT_H

#include "git2.h"

#include <QObject>
#include <QHash>
#include <QDir>

class QGit : public QObject
{
    Q_OBJECT
public:
    explicit QGit(QObject *parent = 0);
    virtual ~QGit();

public slots:
    void repositoryStatus(QDir path);

signals:
    void repositoryStatusReply(QHash<git_status_t, int> items);
    void error(QString qgit_function, QString git_function, int code);
};

#endif // QGIT_H
