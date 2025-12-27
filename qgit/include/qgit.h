#pragma once

#include "qgitdifffile.h"
#include "qgitcommit.h"
#include "qgitbranch.h"
#include "qgiterror.h"

#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QDir>
#include <QUrl>


class QGitDiffWidgetLine
{
public:
    QByteArray content;
    int new_lineno = 0;
    int old_lineno = 0;
    char origin = '\0';
};

class QGitRemote
{
public:
    QString name;
    QString url;
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
    QGit(QGit&& other);
    QGit& operator=(const QGit &other);
    QGit& operator=(QGit &&other);
    ~QGit() = default;

    bool setPath(const QDir &path);
    QDir path();
    QList<QGitRemote> remotes() const;
    QString localBranch() const;
    QList<QString> localBranches() const;

    static QString getBranchNameFromPath(const QString &path);
    static int createLocalRepository(const QDir &path);
    static bool isGitRepository(const QDir &path);
    static bool gitRepositoryDefaultSignature(const QDir &path, QString &name, QString &email);

public slots:
    void init();
    void signature();
    void currentBranch();
    void status();
    void listBranchesAndTags();
    void listStashes();
    void stashSave(QString name);
    void stashRemove(QString name);
    void listChangedFiles(int show, int sort, bool reversed);
    void commitDiff(QString commitId);
    void commitDiffContent(QString first, QString second, QList<QString> files, uint32_t context_lines);
    void stageFiles(QStringList items);
    void unstageFiles(QStringList items);
    void stageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void unstageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines);
    void commit(QString message, bool withPush);
    void clone(QUrl url);
    void pull();
    void fetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags);
    void push(QString remote, QStringList branches, bool tags, bool force);
    void listCommits(QString object, int length);

signals:
    void initReply(QGitError error);
    void signatureReply(QString name, QString email, QGitError error);
    void currentBranchReply(QString name, QGitError error);
    void statusReply(QMap<git_status_t, int> items, QGitError error);
    void listBranchesAndTagsReply(QList<QGitBranch> branches, QList<QString> tags, QGitError error);
    void stashSaveReply(QGitError error);
    void stashRemoveReply(QGitError error);
    void listStashesReply(QStringList stashes, QGitError error);
    void listChangedFilesReply(QMap<QString,git_status_t> files, QGitError error);
    void commitDiffReply(QString commit_id, QGitCommit diff, QGitError error);
    void commitDiffContentReply(QString first, QString second, QList<QGitDiffFile> files, QGitError error);
    void stageFilesReply(QGitError error);
    void unstageFilesReply(QGitError error);
    void commitReply(QString commit_id, QGitError error);
    void cloneReply(QGitError error);
    void cloneProgressReply(QString path, size_t completed_steps, size_t total_steps);
    void cloneTransferReply(unsigned int total_objects, unsigned int indexed_objects, unsigned int received_objects, unsigned int local_objects, unsigned int total_deltas, unsigned int indexed_deltas, size_t received_bytes);
    void pullReply(QGitError error);
    void fetchReply(QGitError error);
    void pushReply(QGitError error);
    void pushProgress(unsigned int current, unsigned int total, size_t bytes);
    void listCommitsReply(QList<QGitCommit> commits, QGitError error);

private:
    QDir m_path;
};
