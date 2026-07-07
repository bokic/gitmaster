#pragma once

#include "qgitdifffile.h"
#include "qgitcommit.h"
#include "qgitbranch.h"
#include "qgiterror.h"
#include "qgittag.h"

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

    static QString getBranchNameFromPath(const QString &path);
    static int createLocalRepository(const QDir &path);
    static bool isGitRepository(const QDir &path);
    static bool gitRepositoryDefaultSignature(const QDir &path, QString &name, QString &email);

public slots:
    void init();
    void signature();
    void updateSubmodule(QString name);
    void deleteBranches(QList<QGitBranch> branches, bool force);
    void status();
    void listBranchesAndTags();
    void listStashes();
    void stashSave(QString name, bool keepIndex = false, bool includeUntracked = false, bool includeIgnored = false);
    void stashRemove(QString name);
    void stashApply(QString name);
    void stashPop(QString name);
    void listChangedFiles(int show, int sort, bool reversed);
    void commitDiff(QString commitId, bool ignoreWhitespace = false);
    void commitDiffContent(QString first, QString second, QList<QString> files, uint32_t context_lines, bool ignoreWhitespace = false);
    void stageFiles(QStringList items);
    void unstageFiles(QStringList items);
    void stageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void unstageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void discardFiles(QStringList items);
    void discardFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void commit(QString message, bool withPush, bool amend = false);
    void clone(QUrl url);
    void pull(QString remote, QString branch, bool rebase);
    void fetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags);
    void push(QString remote, QStringList branches, bool tags, bool force);
    void listCommits(QString object, int length);
    void searchCommits(QString text, QString type);
    void abortSearch();
    void checkoutBranch(QString name);
    void renameBranch(QString oldName, QString newName);
    void renameTag(QString oldName, QString newName);
    void deleteTag(QString name);
    void setUpstream(QString branchName, QString upstreamBranchName);
    void merge(QString branchName);
    void rebase(QString upstream, QString branch = "", QString onto = "");
    void createTag(QString name, QString targetObjectId, QString message, bool force);
    void clean(bool includeIgnored, bool removeDirectories);
    void applyPatch(QString patchPath);
    void setNote(QString commitHash, QString note);
    void removeNote(QString commitHash);

signals:
    void initReply(QGitError error);
    void signatureReply(QString name, QString email, QGitError error);
    void updateSubmoduleReply(QGitError error);
    void statusReply(QMap<git_status_t, int> items, QGitError error);
    void listBranchesAndTagsReply(QList<QGitBranch> branches, QList<QGitTag> tags, QGitError error);
    void stashSaveReply(QGitError error);
    void stashRemoveReply(QGitError error);
    void stashApplyReply(QGitError error);
    void stashPopReply(QGitError error);
    void listStashesReply(QStringList stashes, QGitError error);
    void listChangedFilesReply(QList<QPair<QString, git_status_t>> files, QGitError error);
    void commitDiffReply(QString commit_id, QGitCommit diff, QGitError error);
    void commitDiffContentReply(QString first, QString second, QList<QGitDiffFile> files, QGitError error);
    void stageFilesReply(QGitError error);
    void unstageFilesReply(QGitError error);
    void discardFilesReply(QGitError error);
    void commitReply(QString commit_id, QGitError error);
    void cloneReply(QGitError error);
    void cloneProgressReply(QString path, size_t completed_steps, size_t total_steps);
    void cloneTransferReply(unsigned int total_objects, unsigned int indexed_objects, unsigned int received_objects, unsigned int local_objects, unsigned int total_deltas, unsigned int indexed_deltas, size_t received_bytes);
    void pullReply(QGitError error);
    void fetchReply(QGitError error);
    void pushReply(QGitError error);
    void pushProgress(unsigned int current, unsigned int total, size_t bytes);
    void listCommitsReply(QList<QGitCommit> commits, QGitError error);
    void commitFound(QGitCommit commit);
    void searchFinished();
    void deleteBranchesReply(QGitError error);
    void checkoutBranchReply(QGitError error);
    void renameBranchReply(QGitError error);
    void renameTagReply(QGitError error);
    void createTagReply(QGitError error);
    void setUpstreamReply(QGitError error);
    void deleteTagReply(QGitError error);
    void mergeReply(QGitError error);
    void rebaseReply(QGitError error);
    void cleanReply(QGitError error);
    void applyPatchReply(QGitError error);
    void setNoteReply(QGitError error);
    void removeNoteReply(QGitError error);

private:
    QDir m_path;
    std::atomic<bool> m_abortSearch{false};
};
