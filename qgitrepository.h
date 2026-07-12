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
    void worktreeDialog();
    void gitFlowDialog();
    void commit();
    void fetch();
    void pull();
    void push();
    void merge();
    void rebase();
    QGit *git() const;
    void navigateToCommit(const QString &hash);
    bool hasRemotes() const { return m_hasRemotes; }
    bool hasCommitsToPush() const { return m_hasCommitsToPush; }

signals:
    void localStash(const QString &name, bool keepIndex, bool includeUntracked, bool includeIgnored);
    void repositoryPush(const QString &remote, const QStringList &branches, bool tags, bool force);
    void repositoryPull(const QString &remote, const QString &branch, bool rebase);
    void repositoryFetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags);
    void repositoryBranches();
    void repositoryStashes();
    void repositoryChangedFiles(int show, int sort, bool reversed);
    void repositoryStageFiles(const QStringList &items);
    void repositoryUnstageFiles(const QStringList &items);
    void repositoryCommit(const QString &message, bool withPush, bool amend);
    void repositoryGetCommits(const QString &branchRef, int offset, int length);
    void repositorySearchCommits(const QString &text, const QString &type);
    void repositoryAbortSearch();
    void repositoryGetCommitDiff(const QString &commitId, bool ignoreWhitespace = false);
    void stageFileLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines);
    void unstageFileLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines);
    void repositoryDiscardFiles(const QStringList &items);
    void repositoryDiscardFileLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines);
    void deleteBranches(const QList<QGitBranch> &branches, bool force);
    void repositoryMerge(const QString &branchName);
    void repositoryRebase(const QString &upstream, const QString &branch = "", const QString &onto = "");
    void repositoryRenameBranch(const QString &oldName, const QString &newName);
    void repositoryRenameTag(const QString &oldName, const QString &newName);
    void repositoryCreateTag(const QString &name, const QString &targetObjectId, const QString &message, bool force);
    void repositoryUpdateSubmodule(const QString &name);
    void repositoryClean(bool includeIgnored, bool removeDirectories);
    void repositoryApplyPatch(const QString &patchPath);
    void repositorySetNote(const QString &commitHash, const QString &note);
    void repositoryRemoveNote(const QString &commitHash);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool event(QEvent *event) override;

private slots:
    void gravatarImageDownloadFinished(QNetworkReply *reply);
    void localStashSaveReply(const QGitError &error);
    void localStashRemoveReply(const QGitError &error);
    void repositoryFetchReply(const QGitError &error);
    void repositoryPullReply(const QGitError &error);
    void repositoryPushReply(const QGitError &error);
    void repositoryMergeReply(const QGitError &error);
    void repositoryRebaseReply(const QGitError &error);
    void repositoryUpdateSubmoduleReply(const QGitError &error);
    void repositoryBranchesAndTagsReply(const QList<QGitBranch> &branches, const QList<QGitTag> &tags, const QList<QGitSubmodule> &submodules, const QList<QGitWorktree> &worktrees, bool hasRemotes, bool hasCommitsToPush, const QGitError &error);
    void repositoryStashesReply(const QStringList &stashes, const QGitError &error);
    void repositoryChangedFilesReply(const QList<QPair<QString, git_status_t>> &files, const QGitError &error);
    void repositoryStageFilesReply(const QGitError &error);
    void repositoryUnstageFilesReply(const QGitError &error);
    void repositoryDiscardFilesReply(const QGitError &error);
    void repositoryCleanReply(const QGitError &error);
    void repositoryApplyPatchReply(const QGitError &error);
    void repositorySetNoteReply(const QGitError &error);
    void repositoryRemoveNoteReply(const QGitError &error);
    void repositoryCommitReply(const QString &commit_id, const QGitError &error);
    void repositoryGetCommitsReply(const QList<QGitCommit> &commits, const QGitError &error);
    void repositoryGetCommitDiffReply(const QString &commitId, const QGitCommit &diff, const QGitError &error);
    void deleteBranchesReply(const QGitError &error);
    void historyTableSliderMoved(int pos);
    void selectedLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines);
    void on_repositoryDetail_currentChanged(int index);
    void on_logHistory_commits_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_checkBox_StagedFiles_clicked();
    void on_checkBox_UnstagedFiles_clicked();
    void on_checkBox_amendCommit_clicked(bool checked);
    void on_treeWidget_staged_itemChanged(QTreeWidgetItem *item, int column);
    void on_treeWidget_unstaged_itemChanged(QTreeWidgetItem *item, int column);
    void on_pushButton_commit_clicked();
    void on_pushButton_commitCancel_clicked();
    void on_plainTextEdit_commitMessage_focus();
    void on_logHistory_files_itemSelectionChanged();
    void on_logHistory_files_customContextMenuRequested(const QPoint &pos);
    void on_search_files_customContextMenuRequested(const QPoint &pos);
    void on_treeWidget_staged_itemSelectionChanged();
    void on_treeWidget_unstaged_itemSelectionChanged();
    void on_treeWidget_unstaged_customContextMenuRequested(const QPoint &pos);
    void on_checkBox_PendingFiles_clicked();
    void on_treeWidget_pending_itemChanged(QTreeWidgetItem *item, int column);
    void on_treeWidget_pending_itemSelectionChanged();
    void on_treeWidget_pending_customContextMenuRequested(const QPoint &pos);
    void on_comboBox_gitViewOptions_activated(int index);
    void on_branchesTreeView_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_branchesTreeView_customContextMenuRequested(const QPoint &pos);
    void stashApplyReply(const QGitError &error);
    void stashPopReply(const QGitError &error);
    void checkoutBranchReply(const QGitError &error);
    void renameBranchReply(const QGitError &error);
    void renameTagReply(const QGitError &error);
    void createTagReply(const QGitError &error);
    void setUpstreamReply(const QGitError &error);
    void deleteTagReply(const QGitError &error);
    void on_branchesTreeView_itemChanged(QTreeWidgetItem *item, int column);
    void on_comboBox_gitStatusFiles_itemClicked(int index);
    void on_comboBox_logBranchFilter_currentTextChanged(const QString &text);
    void on_comboBox_gitDiffOptions_optionsChanged();
    void on_commit_diff_customContextMenuRequested(const QPoint &pos);
    void on_logHistory_commits_customContextMenuRequested(const QPoint &pos);
    void on_lineEdit_search_returnPressed();
    void on_search_commits_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_search_files_itemSelectionChanged();
    void onCommitFound(const QGitCommit &commit);
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
    QIcon m_iconWorktree;
    QString m_searchingCommitHash;
    QString m_draftCommitMessage;
    bool m_stageingFiles = true;
    bool m_searchingCommits = false;
    QString m_currentLogBranchFilter;
    int m_logCommitsOffset = 0;
    QString m_lastRemote;
    QStringList m_lastBranches;
    bool m_lastTags;
    QGit *m_git = nullptr;
    bool m_hasRemotes = false;
    bool m_hasCommitsToPush = false;
    int m_layoutOption = 0;
    QList<QPair<QString, git_status_t>> m_changedFiles;
    void updateStatusViews();
    void updateFolderCheckStates(QTreeWidgetItem *item);
    void collectFilePaths(QTreeWidgetItem *item, QStringList &paths);
};
