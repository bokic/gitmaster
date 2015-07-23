#ifndef QGITREPOTREEWIDGET_H
#define QGITREPOTREEWIDGET_H

#include <QTreeWidget>
#include <QThread>
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
};

#endif // QGITREPOTREEWIDGET_H
