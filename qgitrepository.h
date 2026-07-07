#pragma once

#include <QNetworkAccessManager>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QThread>
#include <QWidget>
#include <QIcon>

#include "qgit.h"
#include "qgitcommit.h"
#include "qgitbranch.h"
#include "qgittag.h"


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
    void stash(const QString &name, bool keepIndex = false, bool includeUntracked = false, bool includeIgnored = false);
    void branchDialog();
    void commit();
    void fetch();
    void pull();
    void push();
    void merge();
    QGit *git() const;
    void navigateToCommit(const QString &hash);

signals:
    void localStash(QString name, bool keepIndex, bool includeUntracked, bool includeIgnored);
    void repositoryPush(QString remote, QStringList branches, bool tags, bool force);
    void repositoryPull(QString remote, QString branch, bool rebase);
    void repositoryFetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags);
    void repositoryBranches();
    void repositoryStashes();
    void repositoryChangedFiles(int show, int sort, bool reversed);
    void repositoryStageFiles(QStringList items);
    void repositoryUnstageFiles(QStringList items);
    void repositoryCommit(QString message, bool withPush, bool amend);
    void repositoryGetCommits(QString object, int length);
    void repositorySearchCommits(QString text, QString type);
    void repositoryAbortSearch();
    void repositoryGetCommitDiff(QString commitId, bool ignoreWhitespace = false);
    void stageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void unstageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void repositoryDiscardFiles(QStringList items);
    void repositoryDiscardFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void deleteBranches(QList<QGitBranch> branches, bool force);
    void repositoryMerge(QString branchName);
    void repositoryRebase(QString upstream, QString branch = "", QString onto = "");
    void repositoryRenameBranch(QString oldName, QString newName);
    void repositoryRenameTag(QString oldName, QString newName);
    void repositoryCreateTag(QString name, QString targetObjectId, QString message, bool force);
    void repositoryUpdateSubmodule(QString name);
    void repositoryClean(bool includeIgnored, bool removeDirectories);
    void repositoryApplyPatch(QString patchPath);
    void repositorySetNote(QString commitHash, QString note);
    void repositoryRemoveNote(QString commitHash);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

private slots:
    void gravatarImageDownloadFinished(QNetworkReply *reply);
    void localStashSaveReply(QGitError error);
    void localStashRemoveReply(QGitError error);
    void repositoryFetchReply(QGitError error);
    void repositoryPullReply(QGitError error);
    void repositoryPushReply(QGitError error);
    void repositoryMergeReply(QGitError error);
    void repositoryRebaseReply(QGitError error);
    void repositoryUpdateSubmoduleReply(QGitError error);
    void repositoryBranchesAndTagsReply(QList<QGitBranch> branches, QList<QGitTag> tags, QGitError error);
    void repositoryStashesReply(QStringList stashes, QGitError error);
    void repositoryChangedFilesReply(QList<QPair<QString, git_status_t>> files, QGitError error);
    void repositoryStageFilesReply(QGitError error);
    void repositoryUnstageFilesReply(QGitError error);
    void repositoryDiscardFilesReply(QGitError error);
    void repositoryCleanReply(QGitError error);
    void repositoryApplyPatchReply(QGitError error);
    void repositorySetNoteReply(QGitError error);
    void repositoryRemoveNoteReply(QGitError error);
    void repositoryCommitReply(QString commit_id, QGitError error);
    void repositoryGetCommitsReply(QList<QGitCommit> commits, QGitError error);
    void repositoryGetCommitDiffReply(QString commitId, QGitCommit diff, QGitError error);
    void deleteBranchesReply(QGitError error);
    void historyTableSliderMoved(int pos);
    void selectedLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void on_repositoryDetail_currentChanged(int index);
    void on_logHistory_commits_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_checkBox_StagedFiles_clicked();
    void on_checkBox_UnstagedFiles_clicked();
    void on_checkBox_amendCommit_clicked(bool checked);
    void on_listWidget_staged_itemChanged(QListWidgetItem *item);
    void on_listWidget_unstaged_itemChanged(QListWidgetItem *item);
    void on_pushButton_commit_clicked();
    void on_pushButton_commitCancel_clicked();
    void on_plainTextEdit_commitMessage_focus();
    void on_logHistory_files_itemSelectionChanged();
    void on_logHistory_files_customContextMenuRequested(const QPoint &pos);
    void on_search_files_customContextMenuRequested(const QPoint &pos);
    void on_listWidget_staged_itemSelectionChanged();
    void on_listWidget_unstaged_itemSelectionChanged();
    void on_listWidget_unstaged_customContextMenuRequested(const QPoint &pos);
    void on_branchesTreeView_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_branchesTreeView_customContextMenuRequested(const QPoint &pos);
    void stashApplyReply(QGitError error);
    void stashPopReply(QGitError error);
    void checkoutBranchReply(QGitError error);
    void renameBranchReply(QGitError error);
    void renameTagReply(QGitError error);
    void createTagReply(QGitError error);
    void setUpstreamReply(QGitError error);
    void deleteTagReply(QGitError error);
    void on_branchesTreeView_itemChanged(QTreeWidgetItem *item, int column);
    void on_comboBox_gitStatusFiles_itemClicked(int index);
    void on_comboBox_gitDiffOptions_optionsChanged();
    void on_commit_diff_customContextMenuRequested(const QPoint &pos);
    void on_logHistory_commits_customContextMenuRequested(const QPoint &pos);
    void on_lineEdit_search_returnPressed();
    void on_search_commits_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_search_files_itemSelectionChanged();
    void onCommitFound(QGitCommit commit);
    void onSearchFinished();

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
    QIcon m_iconFileRenamed;
    QIcon m_iconFileConflict;
    QIcon m_iconFileIgnored;
    QIcon m_iconFileUnknown;
    QIcon m_iconTag;
    QIcon m_iconWhiteCheckbox;
    QIcon m_iconCurrentBranch;
    QIcon m_iconBranch;
    QIcon m_iconRemote;
    QIcon m_iconStash;
    QIcon m_iconRemoteBranch;
    QIcon m_iconSubmodule;
    QString m_searchingCommitHash;
    QString m_draftCommitMessage;
    bool m_stageingFiles = true;
    bool m_searchingCommits = false;
    QString m_lastRemote;
    QStringList m_lastBranches;
    bool m_lastTags;
    QGit *m_git = nullptr;
};
