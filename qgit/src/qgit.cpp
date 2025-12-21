#include "qgit.h"
#include "qgitdifffile.h"
#include "qgitdiffhunk.h"
#include "qgitdiffline.h"
#include "qgiterror.h"

#include <QInputDialog>
#include <QThread>
#include <QString>
#include <QVector>
#include <QDir>


static void git_repository_defer_cleanup(git_repository **repo)
{
    git_repository_free(*repo);
    *repo = nullptr;
}

static void git_reference_defer_cleanup(git_reference **ref)
{
    git_reference_free(*ref);
    *ref = nullptr;
}

static void git_signature_defer_cleanup(git_signature **me)
{
    git_signature_free(*me);
    *me = nullptr;
}

static void git_status_list_defer_cleanup(git_status_list **list)
{
    git_status_list_free(*list);
    *list = nullptr;
}

static void git_branch_iterator_defer_cleanup(git_branch_iterator **it)
{
    git_branch_iterator_free(*it);
    *it = nullptr;
}

static void git_tree_defer_cleanup(git_tree **tree)
{
    git_tree_free(*tree);
    *tree = nullptr;
}

static void git_commit_defer_cleanup(git_commit **commit)
{
    git_commit_free(*commit);
    *commit = nullptr;
}

static void git_diff_defer_cleanup(git_diff **diff)
{
    git_diff_free(*diff);
    *diff = nullptr;
}

static void git_remote_defer_cleanup(git_remote **remote)
{
    git_remote_free(*remote);
    *remote = nullptr;
}

static void git_object_defer_cleanup(git_object **obj)
{
    git_object_free(*obj);
    *obj = nullptr;
}

static void git_patch_defer_cleanup(git_patch **patch)
{
    git_patch_free(*patch);
    *patch = nullptr;
}

static void git_index_defer_cleanup(git_index **index)
{
    git_index_free(*index);
    *index = nullptr;
}

static void git_blob_defer_cleanup(git_blob **blob)
{
    git_blob_free(*blob);
    *blob = nullptr;
}

static void git_revwalk_defer_cleanup(git_revwalk **walker)
{
    git_revwalk_free(*walker);
    *walker = nullptr;
}

static void git_strarray_defer_cleanup(git_strarray *strarray)
{
    git_strarray_dispose(strarray);
    *strarray = {nullptr, 0};
}

#define git_repository_defer(var) git_repository* var __attribute__((cleanup(git_repository_defer_cleanup))) = nullptr
#define git_reference_defer(var) git_reference* var __attribute__((cleanup(git_reference_defer_cleanup))) = nullptr
#define git_signature_defer(var) git_signature* var __attribute__((cleanup(git_signature_defer_cleanup))) = nullptr
#define git_status_list_defer(var) git_status_list* var __attribute__((cleanup(git_status_list_defer_cleanup))) = nullptr
#define git_branch_iterator_defer(var) git_branch_iterator* var __attribute__((cleanup(git_branch_iterator_defer_cleanup))) = nullptr
#define git_tree_defer(var) git_tree* var __attribute__((cleanup(git_tree_defer_cleanup))) = nullptr
#define git_commit_defer(var) git_commit* var __attribute__((cleanup(git_commit_defer_cleanup))) = nullptr
#define git_diff_defer(var) git_diff* var __attribute__((cleanup(git_diff_defer_cleanup))) = nullptr
#define git_remote_defer(var) git_remote* var __attribute__((cleanup(git_remote_defer_cleanup))) = nullptr
#define git_object_defer(var) git_object* var __attribute__((cleanup(git_object_defer_cleanup))) = nullptr
#define git_patch_defer(var) git_patch* var __attribute__((cleanup(git_patch_defer_cleanup))) = nullptr
#define git_index_defer(var) git_index* var __attribute__((cleanup(git_index_defer_cleanup))) = nullptr
#define git_blob_defer(var) git_blob* var __attribute__((cleanup(git_blob_defer_cleanup))) = nullptr
#define git_revwalk_defer(var) git_revwalk* var __attribute__((cleanup(git_revwalk_defer_cleanup))) = nullptr
#define git_strarray_defer(var) git_strarray var __attribute__((cleanup(git_strarray_defer_cleanup))) = {nullptr, 0}

#define LINE_END '\n'


QGit::QGit(const QDir &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
{
}

QGit::QGit(QObject *parent)
    : QObject(parent)
{
}

QGit::QGit(QGit&& other)
{
    qSwap(m_path, other.m_path);
}

QGit& QGit::operator=(const QGit &other)
{
    m_path = other.m_path;
    return *this;
}

QGit& QGit::operator=(QGit &&other)
{
    qSwap(m_path, other.m_path);
    return *this;
}

QGit::~QGit()
{
}

bool QGit::setPath(const QDir &path)
{
    m_path = path;

    return true;
}

QDir QGit::path()
{
    return m_path;
}

QList<QString> QGit::remotes() const
{
    QList<QString> ret;

    git_repository_defer(repo);
    git_strarray_defer(remotes);
    int res = 0;

    res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return ret;
    }

    git_remote_list(&remotes, repo);

    for(size_t c = 0; c < remotes.count; c++)
    {
        ret.append(remotes.strings[c]);
    }

    return ret;
}

QString QGit::localBranch() const
{
    QString ret;

    git_repository_defer(repo);
    git_reference_defer(ref);
    const char *branch = nullptr;
    QGitError error;

    try {

        int res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_repository_head(&ref, repo);
        if (res)
        {
            throw QGitError("git_repository_head", res);
        }

        res = git_branch_name(&branch, ref);
        if (res)
        {
            throw QGitError("git_branch_name", res);
        }

        ret = branch;

    } catch(const QGitError &ex) {
        error = ex;
    }

    return ret;
}

QList<QString> QGit::localBranches() const
{
    QList<QString> ret;
    int res = 0;

    git_repository_defer(repo);
    res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
    if (res)
        return ret;

    git_branch_iterator_defer(it);
    res = git_branch_iterator_new(&it, repo, GIT_BRANCH_LOCAL);
    if (res)
        return ret;

    while (1)
    {
        git_branch_t type = GIT_BRANCH_LOCAL;
        git_reference_defer(ref);

        if (git_branch_next(&ref, &type, it) == 0)
        {
            break;
        }

        const char *name = nullptr;
        res = git_branch_name(&name, ref);
        if (res == 0 && name)
        {
            ret.append(QString::fromUtf8(name));
        }
    }

    return ret;
}

QString QGit::getBranchNameFromPath(const QString &path)
{
    int result = 0;
    const char *branch = nullptr;
    QString ret;

    git_repository_defer(repo);
    result = git_repository_open(&repo, path.toUtf8().constData());
    if (result)
        return ret;

    git_reference_defer(ref);
    result = git_repository_head(&ref, repo);
    if (result)
        return ret;

    git_branch_name(&branch, ref);
    ret = branch;

    return ret;
}

int QGit::createLocalRepository(const QDir &path)
{
    git_repository_defer(repo);
    int err = 0;

    err = git_repository_init(&repo, path.absolutePath().toUtf8().constData(), 0);

    return err;
}

bool QGit::isGitRepository(const QDir &path)
{
    git_repository_defer(repo);
    bool ret = false;
    int err = 0;

    err = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (err == 0)
    {
        ret = true;
    }

    return ret;
}

bool QGit::gitRepositoryDefaultSignature(const QDir &path, QString &name, QString &email)
{
    int result = 0;

    git_repository_defer(repo);
    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
        return false;

    git_signature_defer(me);
    result = git_signature_default(&me, repo);
    if (result)
        return false;

    name = me->name;
    email = me->email;

    return true;
}

void QGit::currentBranch()
{
    git_repository_defer(repo);
    git_reference_defer(ref);
    const char *branch = nullptr;
    int res = 0;

    QGitError error;
    QString name;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_repository_head(&ref, repo);
        if (res)
        {
            throw QGitError("git_repository_head", res);
        }

        res = git_branch_name(&branch, ref);
        if (res)
        {
            throw QGitError("git_branch_name", res);
        }

        name = branch;

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit currentBranchReply(name, error);
}

void QGit::init()
{
    git_repository_defer(repo);
    int res = 0;

    QGitError error;

    try {

        res = git_repository_init(&repo, m_path.absolutePath().toUtf8().constData(), 0);
        if(res)
        {
            throw QGitError("git_repository_init", res);
        }

    } catch(const QGitError & ex) {
        error = ex;
    }


    emit initReply(error);
}

void QGit::signature()
{
    git_repository_defer(repo);
    git_signature_defer(me);
    QString email;
    QString name;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_signature_default(&me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        name = QString::fromUtf8(me->name);
        email = QString::fromUtf8(me->email);

    } catch(const QGitError & ex) {
        error = ex;
    }

    emit signatureReply(name, email, error);
}

void QGit::status()
{
    git_repository_defer(repo);
    git_status_list_defer(list);
    QMap<git_status_t, int> items;
    size_t index = 0;
    int res = 0;

    QGitError error;

    try {

        auto ba = m_path.absolutePath().toUtf8();
        res = git_repository_open(&repo, ba.constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_status_list_new(&list, repo, nullptr);
        if (res)
        {
            throw QGitError("git_status_list_new", res);
        }

        while(const git_status_entry *item = git_status_byindex(list, index))
        {
            items[item->status]++;

            index++;
        }

    } catch(const QGitError & ex) {
        error = ex;
    }

    emit statusReply(items, error);
}

void QGit::listBranchesAndTags()
{
    QList<QGitBranch> branches;

    git_branch_t type = GIT_BRANCH_ALL;
    QList<QString> tags;
    int res = 0;


    QGitError error;

    try {

        git_repository_defer(repo);
        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_branch_iterator_defer(it);
        res = git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL);
        if (res)
        {
            throw QGitError("git_branch_iterator_new", res);
        }

        while(1)
        {
            git_reference_defer(ref);

            if (git_branch_next(&ref, &type, it) == 0)
            {
                break;
            }

            const char *ref_name = git_reference_name(ref);

            QGitBranch branch = QGitBranch(ref_name, type);
            branches.append(branch);

            ref_name = nullptr;
        }

        git_strarray_defer(tag_names);
        res = git_tag_list(&tag_names, repo);
        if (res)
        {
            throw QGitError("git_tag_list", res);
        }

        for(size_t c = 0; c < tag_names.count; c++)
        {
            tags.append(QString::fromUtf8(tag_names.strings[c]));
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listBranchesAndTagsReply(branches, tags, error);
}

void QGit::stashSave(QString name)
{
    git_oid stashid;

    QGitError error;

    try {

        git_repository_defer(repo);
        int res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_signature_defer(me);
        res = git_signature_default(&me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        res = git_stash_save(&stashid, repo, me, name.toUtf8().constData(), GIT_STASH_DEFAULT);
        if (res)
        {
            throw QGitError("git_stash_save", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stashSaveReply(error);
}

void QGit::stashRemove(QString name)
{
    Q_UNUSED(name)
}

void QGit::listStashes()
{
    git_repository_defer(repo);
    QStringList stashes;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_stash_foreach(repo, [](size_t index, const char* message, const git_oid *stash_id, void *payload) -> int {

                Q_UNUSED(index)
                Q_UNUSED(stash_id)

                auto list = static_cast<QStringList *>(payload);

                list->append(QString::fromUtf8(message));

                return 0;
            }, &stashes);
        if (res)
        {
            throw QGitError("git_stash_foreach", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listStashesReply(stashes, error);
}

void QGit::listChangedFiles(int show, int sort, bool reversed)
{
    git_repository_defer(repo);
    git_status_list_defer(list);
    QMap<QString,git_status_t> items;
    git_status_options opts = {0, GIT_STATUS_SHOW_INDEX_AND_WORKDIR, 0, {}, nullptr, 0};
    int res = 0;
    size_t index = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        opts.version = GIT_STATUS_OPTIONS_VERSION;
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

        if ((show == QGIT_STATUS_NONE)||(show & QGIT_STATUS_NEW))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;

        if ((show == QGIT_STATUS_NONE)||(show & QGIT_STATUS_IGNORED))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;

        if ((show == QGIT_STATUS_NONE)||(show == QGIT_STATUS_ALL))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_UNMODIFIED | GIT_STATUS_OPT_INCLUDE_UNREADABLE;

        res = git_status_list_new(&list, repo, &opts);
        if (res)
        {
            throw QGitError("git_status_list_new", res);
        }

        while(const git_status_entry *item = git_status_byindex(list, index))
        {
            git_status_t status = item->status;

            if ((item->index_to_workdir == nullptr)&&(item->head_to_index == nullptr))
            {
                throw QGitError("unknown scenario", res);
            }

            if (
                    (item->index_to_workdir)
                    &&
                    (
                        ((show == QGIT_STATUS_NONE)&&(status == GIT_STATUS_CURRENT))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_NEW)       &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_NEW))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_MODIFIED)  &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_MODIFIED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_DELETED)   &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_DELETED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_RENAMED)   &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_RENAMED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_TYPECHANGE)&&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_WT_TYPECHANGE))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_IGNORED)   &&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_IGNORED))
                        ||
                        ((show != QGIT_STATUS_NONE)&&(show & QGIT_STATUS_CONFLICTED)&&(status != GIT_STATUS_CURRENT)&&(status & GIT_STATUS_CONFLICTED))
                        ||
                        ((show == QGIT_STATUS_ALL))
                    )
                )
            {
                items.insert(QString::fromUtf8(item->index_to_workdir->new_file.path), status);
            }

            if ((item->head_to_index)&&(item->head_to_index->status != GIT_DELTA_UNMODIFIED)&&(status))
            {
                items.insert(QString::fromUtf8(item->head_to_index->new_file.path), status);
            }

            index++;
        }

        if (sort != QGIT_SORT_UNSORTED)
        {
            // ...

            if (reversed)
            {
                // ...
            }
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listChangedFilesReply(items, error);
}

void QGit::commitDiff(QString commitId)
{
    const git_diff_delta *delta = nullptr;
    git_repository_defer(repo);
    QList<QGitCommitDiffParent> parents;
    QGitSignature commitAuthor, commitCommiter;
    QDateTime commitTime;
    QString commitMessage;
    QGitCommit commitDiff;
    unsigned int parentCount;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        if (commitId.isEmpty())
        {
            commitId = QStringLiteral("HEAD");
        }

        git_object_defer(obj);
        res = git_revparse_single(&obj, repo, commitId.toUtf8());
        if (res)
        {
            throw QGitError("git_revparse_single", res);
        }

        git_commit_defer(commit);
        res = git_commit_lookup(&commit, repo, git_object_id(obj));
        if (res)
        {
            throw QGitError("git_commit_lookup", res);
        }

        git_tree_defer(commit_tree);
        res = git_commit_tree(&commit_tree, commit);
        if (res)
        {
            throw QGitError("git_commit_tree", res);
        }

        commitAuthor = QGitSignature(QString::fromUtf8(git_commit_author(commit)->name), QString::fromUtf8(git_commit_author(commit)->email), QDateTime::fromMSecsSinceEpoch(git_commit_author(commit)->when.time * 1000));
        commitCommiter = QGitSignature(QString::fromUtf8(git_commit_committer(commit)->name), QString::fromUtf8(git_commit_committer(commit)->email), QDateTime::fromMSecsSinceEpoch(git_commit_committer(commit)->when.time * 1000));

        auto time = git_commit_time(commit);
        auto timeOffset = git_commit_time_offset(commit);
        commitTime = QDateTime::fromMSecsSinceEpoch(time * 1000);
        commitTime.setTimeZone(QTimeZone(timeOffset * 60));

        commitMessage = QString::fromUtf8(git_commit_message(commit));

        parentCount = git_commit_parentcount(commit);

        if (parentCount == 0)
        {
            QGitCommitDiffParent item;

            git_diff_defer(diff);
            res = git_diff_tree_to_tree(&diff, repo, nullptr, commit_tree, nullptr);
            if (res)
            {
                throw QGitError("git_diff_tree_to_tree", res);
            }

            size_t _count = git_diff_num_deltas(diff);
            for(size_t c = 0; c < _count; c++)
            {
                delta = git_diff_get_delta(diff, c);

                item.addFile(delta);
            }

            parents.append(item);
        }
        else
        {
            for(unsigned int c = 0; c < parentCount; c++)
            {
                git_commit_defer(parent);
                res = git_commit_parent(&parent, commit, c);
                if (res)
                {
                    throw QGitError("git_commit_parent", res);
                }

                char commit_id[41] = {0, };
                const git_oid *oid = git_commit_id(parent);
                git_oid_tostr(commit_id, 41, oid);
                QGitCommitDiffParent item(commit_id);

                git_tree_defer(parent_tree);
                res = git_commit_tree(&parent_tree, parent);
                if (res)
                {
                    throw QGitError("git_commit_tree", res);
                }

                git_diff_defer(diff);
                res = git_diff_tree_to_tree(&diff, repo, parent_tree, commit_tree, nullptr);
                if (res)
                {
                    throw QGitError("git_diff_tree_to_tree", res);
                }

                size_t _count = git_diff_num_deltas(diff);
                for(size_t c2 = 0; c2 < _count; c2++)
                {
                    delta = git_diff_get_delta(diff, c2);

                    item.addFile(delta);
                }

                parents.append(item);
            }
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    commitDiff = QGitCommit(commitId, parents, commitTime, commitAuthor, commitCommiter, commitMessage);

    emit commitDiffReply(commitId, commitDiff, error);
}

void QGit::commitDiffContent(QString first, QString second, QList<QString> files)
{
    git_commit *first_commit = nullptr, *second_commit = nullptr;
    git_object *first_obj = nullptr, *second_obj = nullptr;
    git_tree *first_tree = nullptr, *second_tree = nullptr;
    const git_diff_delta *delta = nullptr;
    git_strarray_defer(pathspec);
    git_repository_defer(repo);
    QList<QGitDiffFile> items;
    QList<QByteArray> tmp_files;
    QGitError error;
    int res = 0;

    try {
        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        if (!first.isEmpty())
        {
            res = git_revparse_single(&first_obj, repo, first.toUtf8());
            if (res)
            {
                throw QGitError("git_revparse_single(first)", res);
            }

            res = git_commit_lookup(&first_commit, repo, git_object_id(first_obj));
            if (res)
            {
                throw QGitError("git_commit_lookup(first)", res);
            }

            res = git_commit_tree(&first_tree, first_commit);
            if (res)
            {
                throw QGitError("git_commit_tree(first)", res);
            }
        }

        if (!files.isEmpty())
        {
            pathspec.count = static_cast<size_t>(files.count());
            pathspec.strings = static_cast<char **>(malloc(sizeof(char *) * pathspec.count));
            if (pathspec.strings == nullptr)
            {
                throw QGitError("malloc", 0);
            }

            int c = 0;
            for(const auto &file: files)
            {
                QByteArray ba = file.toUtf8();
                tmp_files.push_back(ba);
                pathspec.strings[c] = strdup(tmp_files.last().data());
                c++;
            }
        }

        git_diff_defer(diff);
        if (second == "staged")
        {
            git_diff_options options;

            res = git_revparse_single(&first_obj, repo, "HEAD^{tree}");
            if (res)
            {
                throw QGitError("git_revparse_single(staged)", res);
            }

            res = git_tree_lookup(&first_tree, repo, git_object_id(first_obj));
            if (res)
            {
                throw QGitError("git_tree_lookup(staged)", res);
            }

            memset(&options, 0, sizeof(options));
            options.version = GIT_DIFF_OPTIONS_VERSION;
            options.flags = GIT_DIFF_INCLUDE_UNTRACKED | GIT_DIFF_SHOW_UNTRACKED_CONTENT;
            options.context_lines = 3;
            options.pathspec = pathspec;

            res = git_diff_tree_to_index(&diff, repo, first_tree, nullptr, &options);
            if (res)
            {
                throw QGitError("git_diff_tree_to_index(staged)", res);
            }
        }
        else if (second == "unstaged")
        {
            git_diff_options options;

            memset(&options, 0, sizeof(options));
            options.version = GIT_DIFF_OPTIONS_VERSION;
            options.flags = GIT_DIFF_INCLUDE_UNTRACKED | GIT_DIFF_RECURSE_UNTRACKED_DIRS | GIT_DIFF_SHOW_UNTRACKED_CONTENT;
            options.context_lines = 3;
            options.pathspec = pathspec;

            res = git_diff_index_to_workdir(&diff, repo, nullptr, &options);
            if (res)
            {
                throw QGitError("git_diff_index_to_workdir", res);
            }
        }
        else
        {
            res = git_revparse_single(&second_obj, repo, second.toUtf8());
            if (res)
            {
                throw QGitError("git_revparse_single(second)", res);
            }

            res = git_commit_lookup(&second_commit, repo, git_object_id(second_obj));
            if (res)
            {
                throw QGitError("git_commit_lookup(second)", res);
            }

            res = git_commit_tree(&second_tree, second_commit);
            if (res)
            {
                throw QGitError("git_commit_tree(second)", res);
            }

            res = git_diff_tree_to_tree(&diff, repo, first_tree, second_tree, nullptr);
            if (res)
            {
                throw QGitError("git_diff_tree_to_tree", res);
            }
        }

        size_t _count = git_diff_num_deltas(diff);
        for(size_t c = 0; c < _count; c++)
        {
            delta = git_diff_get_delta(diff, c);

            QGitDiffFile item(delta);

            if (files.contains(item.new_file().path()))
            {
                git_patch_defer(patch);
                res = git_patch_from_diff(&patch, diff, c);
                if (res)
                {
                    throw QGitError("git_patch_from_diff", res);
                }

                size_t hunks = git_patch_num_hunks(patch);
                for(size_t  hunk_cnt = 0; hunk_cnt < hunks; hunk_cnt++) {
                    const git_diff_hunk *hunk = nullptr;
                    res = git_patch_get_hunk(&hunk, nullptr, patch, hunk_cnt);
                    if (res)
                    {
                        throw QGitError("git_patch_get_hunk", res);
                    }

                    QGitDiffHunk hunkObj(hunk);

                    const git_diff_line *line = nullptr;

                    int lines = git_patch_num_lines_in_hunk(patch, hunk_cnt);
                    if (lines > 0)
                    {
                        for(int  line_cnt = 0; line_cnt < lines; line_cnt++) {
                            res = git_patch_get_line_in_hunk(&line, patch, hunk_cnt, static_cast<size_t>(line_cnt));
                            if (res)
                            {
                                throw QGitError("git_patch_get_line_in_hunk", res);
                            }

                            hunkObj.addLine(line);
                        }
                    }

                    item.addHunk(hunkObj);
                }

                items.append(item);
            }
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit commitDiffContentReply(first, second, items, error);
}

void QGit::stageFiles(QStringList items)
{
    int res = 0;

    QGitError error;

    try {

        if (items.count() == 0)
        {
            throw QGitError();
        }

        git_repository_defer(repo);
        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_index_defer(index);
        res = git_repository_index(&index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        for(const auto &item: items)
        {
            res = git_index_add_bypath(index, item.toUtf8().constData());
            if (res == GIT_ENOTFOUND) { // FIXME: Properly implement stage add/delete files.
                res = git_index_remove_bypath(index, item.toUtf8().constData());
                if (res)
                {
                    throw QGitError("git_index_remove_bypath", res);
                }
            }
            if (res)
            {
                throw QGitError("git_index_add_bypath", res);
            }
        }

        res = git_index_write(index);
        if (res)
        {
            throw QGitError("git_index_write", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stageFilesReply(error);
}

void QGit::unstageFiles(QStringList items)
{
    QVector<QByteArray> tmpStrList;
    git_repository_defer(repo);
    git_strarray(paths);
    int res = 0;

    QGitError error;

    try {

        if (items.count() == 0)
        {
            throw QGitError();
        }

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        paths.count = static_cast<size_t>(items.count());
        paths.strings = static_cast<char **>(malloc(sizeof(char *) * paths.count));
        if (paths.strings == nullptr)
        {
            throw QGitError("malloc", 0);
        }

        tmpStrList.reserve(items.count());

        for(int c = 0; c < items.count(); c++)
        {
            tmpStrList.append(items.at(c).toUtf8());
            paths.strings[c] = const_cast<char *>(tmpStrList.at(c).data());
        }

        git_reference_defer(head);
        git_object_defer(head_commit);
        res = git_repository_head(&head, repo);
        if (res == 0)
        {
            res = git_reference_peel(&head_commit, head, GIT_OBJ_COMMIT);
            if (res)
            {
                throw QGitError("git_reference_peel", res);
            }
        }

        res = git_reset_default(repo, head_commit, &paths);
        if (res)
        {
            throw QGitError("git_reset_default", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit unstageFilesReply(error);
}

void QGit::stageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines)
{
    QByteArray buffer;
    git_repository_defer(repo);
    const git_index_entry *entry = nullptr;
    int res = 0;

    QGitError error;

    try {
        if (lines.count() == 0)
        {
            throw QGitError();
        }

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_index_defer(index);
        res = git_repository_index(&index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        entry = git_index_get_bypath(index, filename.toUtf8().constData(), GIT_INDEX_STAGE_NORMAL);
        if (entry == nullptr)
        {
            throw QGitError("git_index_get_bypath", res);
        }

        git_blob_defer(blob);
        res = git_blob_lookup(&blob, repo, &entry->id);
        if (res)
        {
            throw QGitError("git_blob_lookup", res);
        }

        const char *blob_content = static_cast<const char *>(git_blob_rawcontent(blob));
        git_off_t blob_size = git_blob_rawsize(blob);
        buffer = QByteArray(blob_content, blob_size);
        auto bufferLines = buffer.split(LINE_END);
        int deltaLine = 0;

        for(const auto &line: lines)
        {
            QByteArray content;

            if (line.origin == ' ')
            {
                deltaLine = 0;
                continue;
            }

            switch(line.origin)
            {
            case '-':
                bufferLines.removeAt(line.old_lineno + deltaLine - 1);
                deltaLine--;
                break;
            case '+':
                content = line.content;
                content = content.left(content.length() - 1);
                bufferLines.insert(line.new_lineno - 1, content);
                deltaLine++;
                break;
            default:
                throw QGitError("Unknown operation", 0);
            }
        }
        buffer = bufferLines.join(LINE_END);

        res = git_index_add_from_buffer(index, entry, buffer.constData(), buffer.length());
        if (res)
        {
            throw QGitError("git_index_add_frombuffer", res);
        }

        res = git_index_write(index);
        if (res)
        {
            throw QGitError("git_index_write", res);
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stageFilesReply(error);
}

void QGit::unstageFileLines(QString filename, QVector<QGitDiffWidgetLine> lines)
{
    QByteArray buffer;
    git_repository_defer(repo);
    const git_index_entry *entry = nullptr;
    git_index *index = nullptr;
    int res = 0;

    QGitError error;

    try {
        if (lines.count() == 0)
        {
            throw QGitError();
        }

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_repository_index(&index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        entry = git_index_get_bypath(index, filename.toUtf8().constData(), GIT_INDEX_STAGE_NORMAL);
        if (entry == nullptr)
        {
            throw QGitError("git_index_get_bypath", res);
        }

        git_blob_defer(blob);
        res = git_blob_lookup(&blob, repo, &entry->id);
        if (res)
        {
            throw QGitError("git_blob_lookup", res);
        }

        const char *blob_content = static_cast<const char *>(git_blob_rawcontent(blob));
        git_off_t blob_size = git_blob_rawsize(blob);
        buffer = QByteArray(blob_content, blob_size);
        auto bufferLines = buffer.split(LINE_END);
        int deltaLine = 0;

        for(const auto &line: lines)
        {
            QByteArray content;

            if (line.origin == ' ')
            {
                deltaLine = 0;
                continue;
            }

            switch(line.origin)
            {
            case '+':
                bufferLines.removeAt(line.new_lineno + deltaLine - 1);
                deltaLine--;
                break;
            case '-':
                content = line.content;
                content = content.left(content.length() - 1);
                bufferLines.insert(line.old_lineno + deltaLine - 1, content);
                deltaLine++;
                break;
            default:
                throw QGitError("Unknown operation", 0);
            }
        }
        buffer = bufferLines.join(LINE_END);

        res = git_index_add_from_buffer(index, entry, buffer.constData(), buffer.length());
        if (res)
        {
            throw QGitError("git_index_add_frombuffer", res);
        }

        res = git_index_write(index);
        if (res)
        {
            throw QGitError("git_index_write", res);
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit unstageFilesReply(error);

}

void QGit::commit(QString message)
{
    git_repository_defer(repo);
    git_oid new_commit_id = { {0} };
    git_oid parent_id = { {0} };
    git_oid tree_id = { {0} };
    size_t _count = 0;
    int unborn = 0;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        /// Check if somthing is staged
        git_object_defer(git_obj);
        git_tree_defer(tree);
        res = git_revparse_single(&git_obj, repo, "HEAD^{tree}");
        if (res == 0)
        {
            res = git_tree_lookup(&tree, repo, git_object_id(git_obj));
            if (res)
            {
                throw QGitError("git_tree_lookup(staged)", res);
            }
        }

        git_diff_defer(diff);
        res = git_diff_tree_to_index(&diff, repo, tree, nullptr, nullptr);
        if (res)
        {
            throw QGitError("git_diff_tree_to_index(staged)", res);
        }

        _count = git_diff_num_deltas(diff);

        if (_count == 0)
        {
            throw QGitError("Nothing staged.", res);
        }

        git_signature_defer(me);
        res = git_signature_default(&me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        git_index_defer(index);
        res = git_repository_index(&index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        res = git_index_write_tree(&tree_id, index);
        if (res)
        {
            throw QGitError("git_index_write_tree", res);
        }

        res = git_tree_lookup(&tree, repo, &tree_id);
        if (res)
        {
            throw QGitError("git_tree_lookup", res);
        }

        unborn = git_repository_head_unborn(repo);
        if (unborn)
        {
            res = git_commit_create_v(
              &new_commit_id,
              repo,
              "HEAD",                        /* name of ref to update */
              me,                            /* author */
              me,                            /* committer */
              nullptr,                       /* nullptr = UTF-8 message encoding */
              message.toUtf8().constData(),  /* message */
              tree,                          /* root tree */
              0);                            /* parent count */
            if (res)
            {
                throw QGitError("git_commit_create_v", res);
            }
        }
        else
        {
            res = git_reference_name_to_id(&parent_id, repo, "HEAD");
            if (res)
            {
                throw QGitError("git_reference_name_to_id", res);
            }

            git_commit_defer(parent);
            res = git_commit_lookup(&parent, repo, &parent_id);
            if (res)
            {
                throw QGitError("git_commit_lookup", res);
            }

            res = git_commit_create_v(
              &new_commit_id,
              repo,
              "HEAD",                        /* name of ref to update */
              me,                            /* author */
              me,                            /* committer */
              nullptr,                       /* nullptr = UTF-8 message encoding */
              message.toUtf8().constData(),  /* message */
              tree,                          /* root tree */
              1,                             /* parent count */
              parent);                       /* parent */
            if (res)
            {
                throw QGitError("git_commit_create_v", res);
            }
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit commitReply(QString::fromUtf8(git_oid_tostr_s(&new_commit_id)), error);
}

void QGit::clone(QUrl url)
{
    git_repository_defer(repo);
    git_clone_options opts;
    int res = 0;

    QGitError error;

    try {

        res = git_clone_init_options(&opts, GIT_CLONE_OPTIONS_VERSION);
        if (res)
        {
            throw QGitError("git_clone_init_options", res);
        }

        opts.fetch_opts.callbacks.payload = this;
        opts.fetch_opts.callbacks.transfer_progress = [](const git_transfer_progress *stats, void *payload) -> int {

            auto _this = static_cast<QGit *>(payload);

            emit _this->cloneTransferReply(stats->total_objects, stats->indexed_objects, stats->received_objects, stats->local_objects, stats->total_deltas, stats->indexed_deltas, stats->received_bytes);

            return QThread::currentThread()->isInterruptionRequested();
        };

        opts.checkout_opts.progress_payload = this;
        opts.checkout_opts.progress_cb = [](
                const char *path,
                size_t completed_steps,
                size_t total_steps,
                void *payload)
        {
            auto _this = static_cast<QGit *>(payload);

            emit _this->cloneProgressReply(QString::fromUtf8(path), completed_steps, total_steps);

            _this = nullptr;
        };

        res = git_clone(&repo, url.toString().toUtf8().constData(), m_path.absolutePath().toUtf8().constData(), &opts);
        if (res)
        {
            throw QGitError("git_clone", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit cloneReply(error);
}

void QGit::pull()
{
    git_repository_defer(repo);
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_remote_defer(remote);
        res = git_remote_lookup(&remote, repo, "origin");
        if (res)
        {
            throw QGitError("git_remote_lookup", res);
        }

        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
        fetch_opts.callbacks.credentials = [](git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)
        {
            if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
                auto pass = QInputDialog::getText(nullptr, "SSh password", "Enter password", QLineEdit::Password).toUtf8();
                if (!pass.isEmpty())
                {
                    QString sshDir = QDir::homePath() + QDir::separator() + ".ssh"; 
                    auto keys = QDir(sshDir).entryInfoList({"*.pub"});
                    if (keys.size() == 1)
                    {
                        QFileInfo pubkeyFileInfo = keys.first();
                        QFileInfo privkeyFileInfo(pubkeyFileInfo.dir(), pubkeyFileInfo.completeBaseName());

                        auto pubkeyPathname = pubkeyFileInfo.absoluteFilePath().toUtf8();
                        auto privkeyPathname = privkeyFileInfo.absoluteFilePath().toUtf8();

                        git_credential_ssh_key_new(out, username_from_url, pubkeyPathname, privkeyPathname, pass);

                        return 0;
                    }
                }
            }

            return -1;
        };

        res = git_remote_fetch(remote, nullptr, &fetch_opts, "pull");
        if (res)
        {
            throw QGitError("git_remote_fetch", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit pullReply(error);
}

void QGit::fetch(bool fetchFromAllRemotes, bool purgeDeletedBranches, bool fetchAllTags)
{
    git_repository_defer(repo);
    git_strarray_defer(remotes);
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_remote_list(&remotes, repo);
        if (res)
        {
            throw QGitError("git_remote_list", res);
        }

        QVector<QByteArray> remotesToFetch;

        if (fetchFromAllRemotes)
        {
            remotesToFetch.reserve(static_cast<int>(remotes.count));
            for(size_t idx = 0; idx < remotes.count; idx++)
            {
                remotesToFetch.append(QByteArray(remotes.strings[idx]));
            }
        }
        else
        {
            QByteArray selected;
            for(size_t idx = 0; idx < remotes.count; idx++)
            {
                if (QString::fromUtf8(remotes.strings[idx]) == QStringLiteral("origin"))
                {
                    selected = QByteArray(remotes.strings[idx]);
                    break;
                }
            }

            if (selected.isEmpty() && remotes.count > 0)
            {
                selected = QByteArray(remotes.strings[0]);
            }

            if (!selected.isEmpty())
            {
                remotesToFetch.append(selected);
            }
        }

        if (remotesToFetch.isEmpty())
        {
            throw QGitError("git_remote_lookup", GIT_ENOTFOUND);
        }

        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
        fetch_opts.callbacks.credentials = [](git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)
        {
            if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
                auto pass = QInputDialog::getText(nullptr, "SSh password", "Enter password", QLineEdit::Password).toUtf8();
                if (!pass.isEmpty())
                {
                    QString sshDir = QDir::homePath() + "/.ssh";
                    auto keys = QDir(sshDir).entryInfoList({"*.pub"});
                    if (keys.size() == 1)
                    {
                        QFileInfo pubkeyFileInfo = keys.first();
                        QFileInfo privkeyFileInfo(pubkeyFileInfo.dir(), pubkeyFileInfo.completeBaseName());

                        auto pubkeyPathname = pubkeyFileInfo.absoluteFilePath().toUtf8();
                        auto privkeyPathname = privkeyFileInfo.absoluteFilePath().toUtf8();

                        git_credential_ssh_key_new(out, username_from_url, pubkeyPathname, privkeyPathname, pass);

                        return 0;
                    }
                }
            }

            return -1;
        };
        fetch_opts.prune = purgeDeletedBranches ? GIT_FETCH_PRUNE : GIT_FETCH_NO_PRUNE;
        fetch_opts.download_tags = fetchAllTags ? GIT_REMOTE_DOWNLOAD_TAGS_ALL : GIT_REMOTE_DOWNLOAD_TAGS_AUTO;

        for(const auto &remoteName: remotesToFetch)
        {
            git_remote_defer(remote);
            res = git_remote_lookup(&remote, repo, remoteName.constData());
            if (res)
            {
                throw QGitError("git_remote_lookup", res);
            }

            res = git_remote_fetch(remote, nullptr, &fetch_opts, "fetch");
            if (res)
            {
                throw QGitError("git_remote_fetch", res);
            }
        }

    } catch(const QGitError &ex) {
        fprintf(stderr, "error doing git fetch!\n");
        error = ex;
    }

    emit fetchReply(error);
}

void QGit::push(QString remote, QStringList branches, bool tags, bool force)
{
    git_repository_defer(repo);
    git_strarray_defer(refspecs);

    QGitError error;

    try {

        int res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_remote_defer(libgit2_remote);
        res = git_remote_lookup(&libgit2_remote, repo, remote.toUtf8().constData());
        if (res)
        {
            throw QGitError("git_remote_lookup", res);
        }

        for(const auto &branch: branches)
        {
            git_reference_defer(ref);
            res = git_reference_lookup(&ref, repo, branch.toUtf8().constData());
            if (res)
            {
                throw QGitError("Local branch not found!", res);
            }

            git_reference_defer(upstream_branch_ref);
            res = git_branch_upstream(&upstream_branch_ref, ref);
            if (res)
            {
                throw QGitError("Local branch is not linked with remote branch!", res);
            }
        }

        refspecs.count = branches.size();
        refspecs.strings = static_cast<char **>(malloc(sizeof(char *) * refspecs.count));
        for(int i = 0; i < branches.size(); i++)
        {
            QByteArray refspec = "refs/heads/" + branches[i].toUtf8() + ":refs/heads/" + branches[i].toUtf8();
            if (force) refspec.prepend("+");
            refspecs.strings[i] = strdup(refspec.constData());
        }

        git_push_options push_opts = GIT_PUSH_OPTIONS_INIT;
        push_opts.callbacks.payload = this;
        push_opts.callbacks.push_transfer_progress = [](unsigned int current, unsigned int total, size_t bytes,void *payload)->int
        {
            QGit *_this = static_cast<QGit *>(payload);
            emit _this->pushProgress(current, total, bytes);
            return 0;
        };

        push_opts.callbacks.credentials = [](git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)
        {
            if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
                auto pass = QInputDialog::getText(nullptr, "SSh password", "Enter password", QLineEdit::Password).toUtf8();
                if (!pass.isEmpty())
                {
                    QString sshDir = QDir::homePath() + "/.ssh";
                    auto keys = QDir(sshDir).entryInfoList({"*.pub"});
                    if (keys.size() == 1)
                    {
                        QFileInfo pubkeyFileInfo = keys.first();
                        QFileInfo privkeyFileInfo(pubkeyFileInfo.dir(), pubkeyFileInfo.completeBaseName());

                        auto pubkeyPathname = pubkeyFileInfo.absoluteFilePath().toUtf8();
                        auto privkeyPathname = privkeyFileInfo.absoluteFilePath().toUtf8();

                        git_credential_ssh_key_new(out, username_from_url, pubkeyPathname, privkeyPathname, pass);

                        return 0;
                    }
                }
            }

            return -1;
        };

        res = git_remote_push(libgit2_remote, &refspecs, &push_opts);
        if (res)
        {
            throw QGitError("git_remote_push", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit pushReply(error);
}

void QGit::listCommits(QString object, int length)
{
    QList<QGitCommit> commits;
    git_oid oid;
    int count = 0;
    int res = 0;

    QGitError error;

    try {

        git_repository_defer(repo);
        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_revwalk_defer(walker);
        res = git_revwalk_new(&walker, repo);
        if (res)
        {
            throw QGitError("git_revwalk_new", res);
        }

        if (object.isEmpty())
        {
            res = git_revwalk_push_head(walker);
            if (res)
            {
                length = 0;
            }
        }
        else
        {
            res = git_oid_fromstr(&oid, object.toUtf8().constData());
            if (res)
            {
                throw QGitError("git_oid_fromstr", res);
            }

            res = git_revwalk_push(walker, &oid);
            if (res)
            {
                throw QGitError("git_revwalk_push", res);
            }

            memset(&oid, 0, sizeof(oid));

            git_revwalk_next(&oid, walker);

            memset(&oid, 0, sizeof(oid));
        }

        while ((!git_revwalk_next(&oid, walker))&&(count < length)) {

            QGitCommit item;
            unsigned int parents = 0;

            QString commit_id;
            QList<QGitCommitDiffParent> commit_parents;
            QDateTime commit_time;
            QGitSignature commit_author;
            QGitSignature commit_commiter;
            QString commit_message;

            git_commit_defer(commit);
            res = git_commit_lookup(&commit, repo, &oid);
            if (res)
            {
                throw QGitError("git_commit_lookup", res);
            }

            commit_id = QString::fromUtf8(git_oid_tostr_s(&oid));

            parents = git_commit_parentcount(commit);
            for(unsigned int index = 0; index < parents; index++)
            {
                git_commit_defer(parent);
                QByteArray parentStr;

                res = git_commit_parent(&parent, commit, index);
                if (res)
                {
                    throw QGitError("git_commit_parent", res);
                }

                const git_oid *parent_iod =  git_commit_id(parent);
                parentStr = QByteArray(git_oid_tostr_s(parent_iod));

                commit_parents.append(QGitCommitDiffParent(parentStr));
            }

            auto time = git_commit_time(commit);
            auto timeOffset = git_commit_time_offset(commit);
            commit_time = QDateTime::fromMSecsSinceEpoch(time * 1000);
            commit_time.setTimeZone(QTimeZone(timeOffset * 60));

            QString author_name = QString::fromUtf8(git_commit_author(commit)->name);
            QString author_email = QString::fromUtf8(git_commit_author(commit)->email);
            QDateTime author_when = QDateTime::fromMSecsSinceEpoch(git_commit_author(commit)->when.time * 1000);
            author_when.setTimeZone(QTimeZone(git_commit_author(commit)->when.offset * 60));
            commit_author = QGitSignature(author_name, author_email, author_when);

            QString commiter_name =  QString::fromUtf8(git_commit_committer(commit)->name);
            QString commiter_email = QString::fromUtf8(git_commit_committer(commit)->email);
            QDateTime commiter_when = QDateTime::fromMSecsSinceEpoch(git_commit_committer(commit)->when.time * 1000);
            commiter_when.setTimeZone(QTimeZone(git_commit_committer(commit)->when.offset * 60));
            commit_commiter = QGitSignature(commiter_name, commiter_email, commiter_when);

            commit_message = QString::fromUtf8(git_commit_message(commit));

            item = QGitCommit(commit_id, commit_parents, commit_time, commit_author, commit_commiter, commit_message);

            commits.push_back(item);

            count++;
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listCommitsReply(commits, error);
}
