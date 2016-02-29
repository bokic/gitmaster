#pragma once

#include <QTreeWidget>
#include <QThread>
#include <QDir>
#include "qgit.h"

class QGitRepoTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QGitRepoTreeWidget(QWidget *parent = 0);
    ~QGitRepoTreeWidget();

    void refreshItems();

signals:

public slots:

private:
    QThread m_thread;
    QGit *m_git;

signals:
    void repositoryStatus(QDir path);

private slots:
    void repositoryStatusReply(QDir path, QMap<git_status_t, int> items);
};
