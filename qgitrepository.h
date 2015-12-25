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
    explicit QGitRepository(const QString &path, QWidget *parent);
    ~QGitRepository();

signals:
    void repositoryBranches(QDir path);
    void repositoryChangedFiles(QDir path);

private slots:
    void repositoryBranchesReply(QList<QGitBranch> branches);
    void repositoryChangedFilesReply(QDir path, QMap<QString, git_status_t> files);
    void on_repositoryDetail_currentChanged(int index);

private:
    Ui::QGitRepository *ui;
    QString m_path;
    QGit *m_git;
};

#endif // QGITREPOSITORY_H
