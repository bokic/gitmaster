#pragma once

#include <QNetworkAccessManager>
#include <QListWidgetItem>
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
    void repositoryBranches();
    void repositoryStashes();
    void repositoryChangedFiles();
    void repositoryStageFiles(QStringList items);
    void repositoryUnstageFiles(QStringList items);
    void repositoryCommit(QString message);
    void repositoryGetCommits(QString object, int length);

private slots:
    void gravatarImageDownloadFinished();
    void repositoryBranchesReply(QList<QGitBranch> branches, QGitError error);
    void repositoryStashesReply(QStringList stashes, QGitError error);
    void repositoryChangedFilesReply(QMap<QString, git_status_t> files, QGitError error);
    void repositoryStageFilesReply(QGitError error);
    void repositoryUnstageFilesReply(QGitError error);
    void repositoryCommitReply(QString commit_id, QGitError error);
    void repositoryGetCommitsReply(QList<QGitCommit> commits, QGitError error);
    void on_repositoryDetail_currentChanged(int index);
    void on_checkBox_StagedFiles_clicked();
    void on_checkBox_UnstagedFiles_clicked();
    void on_listWidget_staged_itemChanged(QListWidgetItem *item);
    void on_listWidget_unstaged_itemChanged(QListWidgetItem *item);
    void on_pushButton_commit_clicked();

private:
    Ui::QGitRepository *ui;
    QNetworkAccessManager m_networkManager;
    QString m_path;
    QGit *m_git;
};
