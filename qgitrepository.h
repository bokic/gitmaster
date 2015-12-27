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
    void repositoryStageFiles(QDir path, QStringList items);
    void repositoryUnstageFiles(QDir path, QStringList items);

private slots:
    void repositoryBranchesReply(QList<QGitBranch> branches);
    void repositoryChangedFilesReply(QDir path, QMap<QString, git_status_t> files);
    void repositoryStageFilesReply(QDir path);
    void repositoryUnstageFilesReply(QDir path);
    void on_repositoryDetail_currentChanged(int index);
    void on_checkBox_StagedFiles_clicked();
    void on_checkBox_UnstagedFiles_clicked();

private:
    Ui::QGitRepository *ui;
    QString m_path;
    QGit *m_git;
};

#endif // QGITREPOSITORY_H
