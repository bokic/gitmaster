#pragma once

#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QThread>
#include <QWidget>

#include "qgit.h"
#include "qgitcommit.h"

namespace Ui {
class QGitRepository;
}

class QGitRepository : public QWidget
{
    Q_OBJECT

public:
    explicit QGitRepository(const QString &path, QWidget *parent);
    ~QGitRepository();
    void stash(const QString &name);

signals:
    void localStash(QString name);
    void repositoryBranches();
    void repositoryStashes();
    void repositoryChangedFiles(int show, int sort, bool reversed);
    void repositoryStageFiles(QStringList items);
    void repositoryUnstageFiles(QStringList items);
    void repositoryCommit(QString message);
    void repositoryGetCommits(QString object, int length);
    void repositoryGetCommitDiff(QString commitId);

private slots:
    void gravatarImageDownloadFinished();
    void localStashSaveReply(QGitError error);
    void repositoryBranchesAndTagsReply(QList<QGitBranch> branches, QList<QString> tags, QGitError error);
    void repositoryStashesReply(QStringList stashes, QGitError error);
    void repositoryChangedFilesReply(QMap<QString, git_status_t> files, QGitError error);
    void repositoryStageFilesReply(QGitError error);
    void repositoryUnstageFilesReply(QGitError error);
    void repositoryCommitReply(QString commit_id, QGitError error);
    void repositoryGetCommitsReply(QList<QGitCommit> commits, QGitError error);
    void repositoryGetCommitDiffReply(QString commitId, QGitCommit diff, QGitError error);
    void historyTableSliderMoved(int pos);
    void on_repositoryDetail_currentChanged(int index);
    void on_logHistory_commits_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_checkBox_StagedFiles_clicked();
    void on_checkBox_UnstagedFiles_clicked();
    void on_listWidget_staged_itemChanged(QListWidgetItem *item);
    void on_listWidget_unstaged_itemChanged(QListWidgetItem *item);
    void on_pushButton_commit_clicked();
    void on_pushButton_commitCancel_clicked();
    void on_plainTextEdit_commitMessage_focus();
    void on_logHistory_files_itemSelectionChanged();
    void on_listWidget_staged_itemSelectionChanged();
    void on_listWidget_unstaged_itemSelectionChanged();
    void on_comboBox_gitStatusFiles_itemClicked(QListWidgetItem *item);

private:
    void fetchCommits();
    void activateCommitOperation(bool activate);
    void fetchRepositoryChangedFiles();
    Ui::QGitRepository *ui;
    QNetworkAccessManager m_networkManager;
    QString m_path;
    bool m_allCommitsLoaded;
    QGitCommit m_stagedDiff;
    QGitCommit m_unstagedDiff;
    QGitCommit m_commitDiff;
    QThread m_thread;
    QGit *m_git;
};
