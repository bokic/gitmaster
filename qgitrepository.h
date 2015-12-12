#ifndef QGITREPOSITORY_H
#define QGITREPOSITORY_H

#include <QWidget>
#include "qgit.h"

namespace Ui {
class QGitRepository;
}

class QGitRepository : public QWidget
{
    Q_OBJECT

public:
    explicit QGitRepository(QWidget *parent = 0);
    ~QGitRepository();

signals:
    void repositoryBranches(QDir path);

private slots:
    void repositoryBranchesReply(QList<QGitBranch> branches);

private:
    Ui::QGitRepository *ui;
    QGit *m_git;
};

#endif // QGITREPOSITORY_H
