#pragma once

#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QThread>
#include <QWidget>
#include <QIcon>

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
    void refreshData();
    void stash(const QString &name);
    void fetch();
    void pull();
    void push();
    QGit *git() const;

signals:
    void localStash(QString name);
    void repositoryPush(QString remote, QStringList branches, bool tags, bool force);
    void repositoryFetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags);
    void repositoryBranches();
    void repositoryStashes();
    void repositoryChangedFiles(int show, int sort, bool reversed);
    void repositoryStageFiles(QStringList items);
    void repositoryUnstageFiles(QStringList items);
    void repositoryCommit(QString message);
    void repositoryGetCommits(QString object, int length);
    void repositoryGetCommitDiff(QString commitId);
    void stageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void unstageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);

protected:
    bool event(QEvent *event) override;

private slots:
    void gravatarImageDownloadFinished(QNetworkReply *reply);
    void localStashSaveReply(QGitError error);
    void repoitoryFetchReply(QGitError error);
    void repoitoryPushReply(QGitError error);
    void repositoryBranchesAndTagsReply(QList<QGitBranch> branches, QList<QString> tags, QGitError error);
    void repositoryStashesReply(QStringList stashes, QGitError error);
    void repositoryChangedFilesReply(QMap<QString, git_status_t> files, QGitError error);
    void repositoryStageFilesReply(QGitError error);
    void repositoryUnstageFilesReply(QGitError error);
    void repositoryCommitReply(QString commit_id, QGitError error);
    void repositoryGetCommitsReply(QList<QGitCommit> commits, QGitError error);
    void repositoryGetCommitDiffReply(QString commitId, QGitCommit diff, QGitError error);
    void historyTableSliderMoved(int pos);
    void selectedLines(QString filename, QVector<QGitDiffWidgetLine> lines);
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
    void on_comboBox_gitStatusFiles_itemClicked(int index);

private:
    void fetchCommits();
    void activateCommitOperation(bool activate);
    void fetchRepositoryChangedFiles();
    Ui::QGitRepository *ui = nullptr;
    QNetworkAccessManager m_networkManager;
    QString m_path;
    bool m_allCommitsLoaded;
    QGitCommit m_stagedDiff;
    QGitCommit m_unstagedDiff;
    QGitCommit m_commitDiff;
    QThread m_thread;
    QIcon m_iconFileNew;
    QIcon m_iconFileClean;
    QIcon m_iconFileModified;
    QIcon m_iconFileRemoved;
    QIcon m_iconFileIgnored;
    QIcon m_iconFileUnknown;
    QIcon m_iconTag;
    QIcon m_iconWhiteCheckbox;
    QIcon m_iconCurrentBranch;
    QIcon m_iconBranch;
    QIcon m_iconRemote;
    QIcon m_iconRemoteBranch;
    bool m_stageingFiles = true;
    QGit *m_git = nullptr;
};
