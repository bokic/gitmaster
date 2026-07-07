#pragma once

#include "qgitdifffile.h"
#include "qgitcommit.h"
#include "qgitbranch.h"
#include "qgiterror.h"
#include "qgittag.h"
#include "qgitworktree.h"

#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QDir>
#include <QUrl>
#include <atomic>


class QGitDiffWidgetLine
{
public:
    QByteArray content;
    int new_lineno = 0;
    int old_lineno = 0;
    int hunk_new_start = 0;
    int hunk_old_start = 0;
    char origin = '\0';
};

class QGitReflogEntry
{
public:
    QString commitHash;
    QString committerName;
    QString committerEmail;
    int64_t time = 0;
    QString message;
};

class QGitBlameHunk
{
public:
    // Range
    size_t linesInHunk = 0;      // Number of lines this hunk covers
    size_t finalStartLineNumber = 0; // 1-based line in the final file

    // Commit info
    QString commitHash;
    QString authorName;
    QString authorEmail;
    int64_t authorTime = 0;      // Unix timestamp
    QString summary;             // Commit summary (first line of message)

    // Original file
    QString origPath;            // Filename in the originating commit
    size_t origStartLineNumber = 0;
};

class QGitRemote
{
public:
    QString name;
    QString url;
};

class QGitSubmodule
{
public:
    QString name;
    QString path;
    QString url;
    QString headId;
    QString indexId;
    QString wdId;
    uint32_t status = 0;
};

class QGit : public QObject
{
    Q_OBJECT
public:
    enum QGitFileStatus {
        QGIT_STATUS_NONE       = 0,
        QGIT_STATUS_NEW        = (1U << 0),
        QGIT_STATUS_MODIFIED   = (1U << 1),
        QGIT_STATUS_DELETED    = (1U << 2),
        QGIT_STATUS_RENAMED    = (1U << 3),
        QGIT_STATUS_TYPECHANGE = (1U << 4),
        QGIT_STATUS_IGNORED    = (1U << 5),
        QGIT_STATUS_CONFLICTED = (1U << 6),
        QGIT_STATUS_ALL        = -1,
    };
    enum QGitFileSort {
        QGIT_SORT_UNSORTED,
        QGIT_SORT_FILENAME,
        QGIT_SORT_FILEPATH,
        QGIT_SORT_STATUS,
        QGIT_SORT_CHECKED
    };

    explicit QGit(const QDir &path, QObject *parent = nullptr);
    explicit QGit(QObject *parent = nullptr);
    ~QGit() = default;

    bool setPath(const QDir &path);
    QDir path();
    QList<QGitRemote> remotes() const;
    bool hasCommitsToPush() const;
    QString currentBranch() const;
    QString headCommitId() const;
    QString headCommitMessage() const;
    bool isAncestor(const QString &ancestor, const QString &descendant) const;
    QString getNote(const QString &commitHash) const;
    QList<QGitReflogEntry> getReflog(const QString &refName = QStringLiteral("HEAD")) const;
    QList<QGitBlameHunk> blameFile(const QString &filePath, const QString &commitId = QString()) const;
    QList<QGitBranch> branches(git_branch_t type) const;
    bool hasCommitId(const QString &commitId) const;
    void createLocalBranch(const QString &name, const QString &commit_id  = "", bool checkout = false, bool force = false);
    void cherrypick(const QString &commitId);
    void revert(const QString &commitId);
    void reset(const QString &commitId, git_reset_t type);
    QString configString(const QString &key) const;
    void setConfigString(const QString &key, const QString &value, bool global = false);
    void deleteConfigEntry(const QString &key, bool global = false);
    QMap<QString, QString> configEntries() const;
    bool hasConflicts() const;
    QList<QString> conflictedFiles() const;
    bool conflictContents(const QString &path, QString &ancestor, QString &ours, QString &theirs) const;
    void resolveConflict(const QString &path, const QString &resolvedContent);
    QList<QGitSubmodule> submodules() const;
    void initSubmodule(const QString &name);
    void syncSubmodule(const QString &name);
    void addRemote(const QString &name, const QString &url);
    void deleteRemote(const QString &name);
    void renameRemote(const QString &oldName, const QString &newName);
    void setRemoteUrl(const QString &name, const QString &url, bool isPushUrl = false);
    QList<QGitWorktree> worktrees() const;
    void addWorktree(const QString &name, const QString &path, const QString &branch, bool newBranch = false);
    void removeWorktree(const QString &name);
    void lockWorktree(const QString &name, bool lock);

    static QString getBranchNameFromPath(const QString &path);
    static int createLocalRepository(const QDir &path);
    static bool isGitRepository(const QDir &path);
    static bool gitRepositoryDefaultSignature(const QDir &path, QString &name, QString &email);

public slots:
    void init();
    void signature();
    void updateSubmodule(const QString &name);
    void deleteBranches(const QList<QGitBranch> &branches, bool force);
    void status();
    void listBranchesAndTags();
    void listStashes();
    void stashSave(const QString &name, bool keepIndex = false, bool includeUntracked = false, bool includeIgnored = false);
    void stashRemove(const QString &name);
    void stashApply(const QString &name);
    void stashPop(const QString &name);
    void listChangedFiles(int show, int sort, bool reversed);
    void commitDiff(const QString &commitId, bool ignoreWhitespace = false);
    void commitDiffContent(const QString &first, const QString &second, const QList<QString> &files, uint32_t context_lines, bool ignoreWhitespace = false);
    void stageFiles(const QStringList &items);
    void unstageFiles(const QStringList &items);
    void stageFileLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines);
    void unstageFileLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines);
    void discardFiles(const QStringList &items);
    void discardFileLines(const QString &filename, const QVector<QGitDiffWidgetLine> &lines);
    void commit(const QString &message, bool withPush, bool amend = false);
    void clone(const QUrl &url);
    void pull(const QString &remote, const QString &branch, bool rebase);
    void fetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags);
    void push(const QString &remote, const QStringList &branches, bool tags, bool force);
    void listCommits(const QString &branchRef, int offset, int length);
    void searchCommits(const QString &text, const QString &type);
    void abortSearch();
    void checkoutBranch(const QString &name);
    void renameBranch(const QString &oldName, const QString &newName);
    void renameTag(const QString &oldName, const QString &newName);
    void deleteTag(const QString &name);
    void setUpstream(const QString &branchName, const QString &upstreamBranchName);
    void merge(const QString &branchName);
    void rebase(const QString &upstream, const QString &branch = "", const QString &onto = "");
    void createTag(const QString &name, const QString &targetObjectId, const QString &message, bool force);
    void clean(bool includeIgnored, bool removeDirectories);
    void applyPatch(const QString &patchPath);
    void setNote(const QString &commitHash, const QString &note);
    void removeNote(const QString &commitHash);

signals:
    void initReply(const QGitError &error);
    void signatureReply(const QString &name, const QString &email, const QGitError &error);
    void updateSubmoduleReply(const QGitError &error);
    void statusReply(const QMap<git_status_t, int> &items, const QGitError &error);
    void listBranchesAndTagsReply(const QList<QGitBranch> &branches, const QList<QGitTag> &tags, bool hasRemotes, bool hasCommitsToPush, const QGitError &error);
    void stashSaveReply(const QGitError &error);
    void stashRemoveReply(const QGitError &error);
    void stashApplyReply(const QGitError &error);
    void stashPopReply(const QGitError &error);
    void listStashesReply(const QStringList &stashes, const QGitError &error);
    void listChangedFilesReply(const QList<QPair<QString, git_status_t>> &files, const QGitError &error);
    void commitDiffReply(const QString &commit_id, const QGitCommit &diff, const QGitError &error);
    void commitDiffContentReply(const QString &first, const QString &second, const QList<QGitDiffFile> &files, const QGitError &error);
    void stageFilesReply(const QGitError &error);
    void unstageFilesReply(const QGitError &error);
    void discardFilesReply(const QGitError &error);
    void commitReply(const QString &commit_id, const QGitError &error);
    void cloneReply(const QGitError &error);
    void cloneProgressReply(const QString &path, size_t completed_steps, size_t total_steps);
    void cloneTransferReply(unsigned int total_objects, unsigned int indexed_objects, unsigned int received_objects, unsigned int local_objects, unsigned int total_deltas, unsigned int indexed_deltas, size_t received_bytes);
    void pullReply(const QGitError &error);
    void fetchReply(const QGitError &error);
    void pushReply(const QGitError &error);
    void pushProgress(unsigned int current, unsigned int total, size_t bytes);
    void listCommitsReply(const QList<QGitCommit> &commits, const QGitError &error);
    void commitFound(const QGitCommit &commit);
    void searchFinished();
    void deleteBranchesReply(const QGitError &error);
    void checkoutBranchReply(const QGitError &error);
    void renameBranchReply(const QGitError &error);
    void renameTagReply(const QGitError &error);
    void createTagReply(const QGitError &error);
    void setUpstreamReply(const QGitError &error);
    void deleteTagReply(const QGitError &error);
    void mergeReply(const QGitError &error);
    void rebaseReply(const QGitError &error);
    void cleanReply(const QGitError &error);
    void applyPatchReply(const QGitError &error);
    void setNoteReply(const QGitError &error);
    void removeNoteReply(const QGitError &error);

private:
    QDir m_path;
    std::atomic<bool> m_abortSearch{false};
};
