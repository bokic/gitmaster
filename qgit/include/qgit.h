#pragma once

//#include "git2.h"
#include "qgitdifffile.h"
#include "qgitcommit.h"
#include "qgitbranch.h"
#include "qgiterror.h"

#include <QReadWriteLock>
#include <QStringList>
#include <QDateTime>
#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QDir>
#include <QUrl>


class QGit : public QObject
{
    Q_OBJECT
public:
    enum QGitFileStatus {
        QGIT_STATUS_NONE       = 0,
        QGIT_STATUS_NEW        = (1u << 0),
        QGIT_STATUS_MODIFIED   = (1u << 1),
        QGIT_STATUS_DELETED    = (1u << 2),
        QGIT_STATUS_RENAMED    = (1u << 3),
        QGIT_STATUS_TYPECHANGE = (1u << 4),
        QGIT_STATUS_IGNORED    = (1u << 5),
        QGIT_STATUS_CONFLICTED = (1u << 6),
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
    virtual ~QGit();

    bool setPath(const QDir &path);
    QDir path() const;
    void abort();
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
    void commitDiffContent(QString first, QString second, QList<QString> files);
    void stageFiles(QStringList items);
    void unstageFiles(QStringList items);
    void commit(QString message);
    void clone(QUrl url);
    void pull();
    void fetch();
    void push();
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
    void listCommitsReply(QList<QGitCommit> commits, QGitError error);

private:
    QReadWriteLock m_pathLock;
    QDir m_path;

    volatile int m_abort = 0;
};
