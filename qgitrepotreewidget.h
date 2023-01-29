#pragma once

#include <QTreeWidget>
#include <QThread>
#include <QDir>
#include "qgit.h"

class QGitRepoTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit QGitRepoTreeWidget(QWidget *parent = nullptr);
    ~QGitRepoTreeWidget();

    void refreshItems();

signals:

public slots:

private:
    void refreshItem();
    QThread m_thread;
    QGit *m_git = nullptr;
    int m_refreshIndex;

signals:
    void repositoryStatus();

private slots:
    void repositoryStatusReply(QMap<git_status_t, int> items, QGitError error);
};
