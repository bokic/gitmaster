#include <qgit.h>
#include <qgitmastermainwindow.h>
#include <qgitdifffile.h>
#include <qgitdiffhunk.h>
#include <qgitdiffline.h>
#include <qgiterror.h>

#include <QThread>
#include <QString>
#include <QVector>
#include <QDir>


struct GitRepository {
    operator git_repository*() { return value; }
    operator git_repository**() { return &value; }
    ~GitRepository() { if (value) { git_repository_free(value); value = nullptr; }}
    git_repository *value = nullptr;
};

struct GitReference {
    operator git_reference*() { return value; }
    operator git_reference**() { return &value; }
    ~GitReference() { if (value) { git_reference_free(value); value = nullptr; }}
    git_reference *value = nullptr;
};

struct GitSignature {
    operator git_signature*() { return value; }
    operator git_signature**() { return &value; }
    ~GitSignature() { if (value) { git_signature_free(value); value = nullptr; }}
    git_signature *value = nullptr;
};

struct GitStatusList {
    operator git_status_list*() { return value; }
    operator git_status_list**() { return &value; }
    ~GitStatusList() { if (value) { git_status_list_free(value); value = nullptr; }}
    git_status_list *value = nullptr;
};

struct GitBranchIterator {
    operator git_branch_iterator*() { return value; }
    operator git_branch_iterator**() { return &value; }
    ~GitBranchIterator() { if (value) { git_branch_iterator_free(value); value = nullptr; }}
    git_branch_iterator *value = nullptr;
};

struct GitTree {
    operator git_tree*() { return value; }
    operator git_tree**() { return &value; }
    ~GitTree() { if (value) { git_tree_free(value); value = nullptr; }}
    git_tree *value = nullptr;
};

struct GitCommit {
    operator git_commit*() { return value; }
    operator git_commit**() { return &value; }
    ~GitCommit() { if (value) { git_commit_free(value); value = nullptr; }}
    git_commit *value = nullptr;
};

struct GitDiff {
    operator git_diff*() { return value; }
    operator git_diff**() { return &value; }
    ~GitDiff() { if (value) { git_diff_free(value); value = nullptr; }}
    git_diff *value = nullptr;
};

struct GitRemote {
    operator git_remote*() { return value; }
    operator git_remote**() { return &value; }
    ~GitRemote() { if (value) { git_remote_free(value); value = nullptr; }}
    git_remote *value = nullptr;
};

struct GitObject {
    operator git_object*() { return value; }
    operator git_object**() { return &value; }
    ~GitObject() { if (value) { git_object_free(value); value = nullptr; }}
    git_object *value = nullptr;
};

struct GitPatch {
    operator git_patch*() { return value; }
    operator git_patch**() { return &value; }
    ~GitPatch() { if (value) { git_patch_free(value); value = nullptr; }}
    git_patch *value = nullptr;
};

struct GitIndex {
    operator git_index*() { return value; }
    operator git_index**() { return &value; }
    ~GitIndex() { if (value) { git_index_free(value); value = nullptr; }}
    git_index *value = nullptr;
};

struct GitBlob {
    operator git_blob*() { return value; }
    operator git_blob**() { return &value; }
    ~GitBlob() { if (value) { git_blob_free(value); value = nullptr; }}
    git_blob *value = nullptr;
};

struct GitRevWalk {
    operator git_revwalk*() { return value; }
    operator git_revwalk**() { return &value; }
    ~GitRevWalk() { if (value) { git_revwalk_free(value); value = nullptr; }}
    git_revwalk *value = nullptr;
};

struct GitStrArray {
    operator git_strarray&() { return value; }
    operator git_strarray*() { return &value; }
    ~GitStrArray() {  git_strarray_dispose(&value); value = {nullptr, 0}; }
    git_strarray value = {nullptr, 0};
};

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
    std::swap(m_path, other.m_path);
}

QGit& QGit::operator=(const QGit &other)
{
    m_path = other.m_path;
    return *this;
}

QGit& QGit::operator=(QGit &&other)
{
    std::swap(m_path, other.m_path);
    return *this;
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

    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return ret;
    }

    GitStrArray remotes;
    git_remote_list(remotes, repo);

    for(size_t c = 0; c < remotes.value.count; c++)
    {
        ret.append(remotes.value.strings[c]);
    }

    return ret;
}

QString QGit::localBranch() const
{
    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return QString();
    }

    GitReference ref;
    res = git_repository_head(ref, repo);
    if (res)
    {
        return QString();
    }

    const char *branch = nullptr;
    res = git_branch_name(&branch, ref);
    if (res)
    {
        return QString();
    }

    return branch;
}

QList<QString> QGit::localBranches() const
{
    QList<QString> ret;

    GitRepository repo;
    int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
    if (res)
    {
        return ret;
    }

    GitBranchIterator it;
    res = git_branch_iterator_new(it, repo, GIT_BRANCH_LOCAL);
    if (res)
    {
        return ret;
    }

    while (1)
    {
        git_branch_t type = GIT_BRANCH_LOCAL;

        GitReference ref;
        if (git_branch_next(ref, &type, it) != 0)
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
    QString ret;

    GitRepository repo;
    int res = git_repository_open(repo, path.toUtf8().constData());
    if (res)
    {
        return ret;
    }

    GitReference ref;
    res = git_repository_head(ref, repo);
    if (res)
    {
        return ret;
    }

    const char *branch = nullptr;
    git_branch_name(&branch, ref);

    return branch;
}

int QGit::createLocalRepository(const QDir &path)
{
    GitRepository repo;
    int res = git_repository_init(repo, path.absolutePath().toUtf8().constData(), 0);

    return res;
}

bool QGit::isGitRepository(const QDir &path)
{
    GitRepository repo;
    int res = git_repository_open(repo, path.absolutePath().toUtf8().constData());
    if (res)
    {
        return false;
    }

    return true;
}

bool QGit::gitRepositoryDefaultSignature(const QDir &path, QString &name, QString &email)
{
    GitRepository repo;
    int res = git_repository_open(repo, path.absolutePath().toUtf8().constData());
    if (res)
    {
        return false;
    }

    GitSignature me;
    res = git_signature_default(me, repo);
    if (res)
    {
        return false;
    }

    name = me.value->name;
    email = me.value->email;

    return true;
}

void QGit::currentBranch()
{
    QGitError error;
    QString name;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitReference ref;
        res = git_repository_head(ref, repo);
        if (res)
        {
            throw QGitError("git_repository_head", res);
        }

        const char *branch = nullptr;
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
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_init(repo, m_path.absolutePath().toUtf8().constData(), 0);
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
    QGitError error;
    QString email;
    QString name;    

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        name = QString::fromUtf8(me.value->name);
        email = QString::fromUtf8(me.value->email);

    } catch(const QGitError & ex) {
        error = ex;
    }

    emit signatureReply(name, email, error);
}

void QGit::status()
{
    QMap<git_status_t, int> items;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitStatusList list;
        res = git_status_list_new(list, repo, nullptr);
        if (res)
        {
            throw QGitError("git_status_list_new", res);
        }

        size_t index = 0;
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

    QList<QString> tags;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitBranchIterator it;
        res = git_branch_iterator_new(it, repo, GIT_BRANCH_ALL);
        if (res)
        {
            throw QGitError("git_branch_iterator_new", res);
        }

        while(1)
        {
            git_branch_t type = GIT_BRANCH_ALL;

            GitReference ref;
            if (git_branch_next(ref, &type, it) != 0)
            {
                break;
            }

            if (git_reference_type(ref) == GIT_REFERENCE_DIRECT)
            {
                const char *ref_name = git_reference_name(ref);
                QGitBranch branch = QGitBranch(ref_name, type);
                branches.append(branch);
            }
        }

        GitStrArray tag_names;
        res = git_tag_list(tag_names, repo);
        if (res)
        {
            throw QGitError("git_tag_list", res);
        }

        for(size_t c = 0; c < tag_names.value.count; c++)
        {
            tags.append(QString::fromUtf8(tag_names.value.strings[c]));
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listBranchesAndTagsReply(branches, tags, error);
}

void QGit::stashSave(QString name)
{

    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        git_oid stashid;
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
    QStringList stashes;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
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
    QMap<QString,git_status_t> items;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        git_status_options opts = {0, GIT_STATUS_SHOW_INDEX_AND_WORKDIR, 0, {}, nullptr, 0};
        opts.version = GIT_STATUS_OPTIONS_VERSION;
        opts.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
        opts.flags = GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX | GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

        if ((show == QGIT_STATUS_NONE)||(show & QGIT_STATUS_NEW))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_UNTRACKED;

        if ((show == QGIT_STATUS_NONE)||(show & QGIT_STATUS_IGNORED))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_IGNORED;

        if ((show == QGIT_STATUS_NONE)||(show == QGIT_STATUS_ALL))
            opts.flags |= GIT_STATUS_OPT_INCLUDE_UNMODIFIED | GIT_STATUS_OPT_INCLUDE_UNREADABLE;

        GitStatusList list;
        res = git_status_list_new(list, repo, &opts);
        if (res)
        {
            throw QGitError("git_status_list_new", res);
        }

        size_t index = 0;
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
            // TODO: Implement sort

            if (reversed)
            {
                // TODO: Implement reversed
            }
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listChangedFilesReply(items, error);
}

void QGit::commitDiff(QString commitId)
{
    QList<QGitCommitDiffParent> parents;
    QGitSignature commitAuthor, commitCommiter;
    QDateTime commitTime;
    QString commitMessage;
    QGitCommit commitDiff;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        if (commitId.isEmpty())
        {
            commitId = QStringLiteral("HEAD");
        }

        GitObject obj;
        res = git_revparse_single(obj, repo, commitId.toUtf8());
        if (res)
        {
            throw QGitError("git_revparse_single", res);
        }

        GitCommit commit;
        res = git_commit_lookup(commit, repo, git_object_id(obj));
        if (res)
        {
            throw QGitError("git_commit_lookup", res);
        }

        GitTree commit_tree;
        res = git_commit_tree(commit_tree, commit);
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

        unsigned int parentCount = git_commit_parentcount(commit);
        if (parentCount == 0)
        {
            QGitCommitDiffParent item;

            GitDiff diff;
            res = git_diff_tree_to_tree(diff, repo, nullptr, commit_tree, nullptr);
            if (res)
            {
                throw QGitError("git_diff_tree_to_tree", res);
            }

            size_t _count = git_diff_num_deltas(diff);
            for(size_t c = 0; c < _count; c++)
            {
                const git_diff_delta *delta = nullptr;
                delta = git_diff_get_delta(diff, c);
                item.addFile(delta);
            }

            parents.append(item);
        }
        else
        {
            for(unsigned int c = 0; c < parentCount; c++)
            {
                GitCommit parent;
                res = git_commit_parent(parent, commit, c);
                if (res)
                {
                    throw QGitError("git_commit_parent", res);
                }

                char commit_id[41] = {0, };
                const git_oid *oid = git_commit_id(parent);
                git_oid_tostr(commit_id, 41, oid);
                QGitCommitDiffParent item(commit_id);

                GitTree parent_tree;
                res = git_commit_tree(parent_tree, parent);
                if (res)
                {
                    throw QGitError("git_commit_tree", res);
                }

                GitDiff diff;
                res = git_diff_tree_to_tree(diff, repo, parent_tree, commit_tree, nullptr);
                if (res)
                {
                    throw QGitError("git_diff_tree_to_tree", res);
                }

                size_t _count = git_diff_num_deltas(diff);
                for(size_t c2 = 0; c2 < _count; c2++)
                {
                    const git_diff_delta *delta = nullptr;
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

void QGit::commitDiffContent(QString first, QString second, QList<QString> files, uint32_t context_lines)
{
    QList<QGitDiffFile> items;
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        if (!first.isEmpty())
        {
            git_object *first_obj = nullptr;
            res = git_revparse_single(&first_obj, repo, first.toUtf8());
            if (res)
            {
                throw QGitError("git_revparse_single(first)", res);
            }

            git_commit *first_commit = nullptr;
            res = git_commit_lookup(&first_commit, repo, git_object_id(first_obj));
            if (res)
            {
                throw QGitError("git_commit_lookup(first)", res);
            }

            git_tree *first_tree = nullptr;
            res = git_commit_tree(&first_tree, first_commit);
            if (res)
            {
                throw QGitError("git_commit_tree(first)", res);
            }
        }

        GitStrArray pathspec;
        if (!files.isEmpty())
        {
            pathspec.value.count = static_cast<size_t>(files.count());
            pathspec.value.strings = static_cast<char **>(malloc(sizeof(char *) * pathspec.value.count));
            if (pathspec.value.strings == nullptr)
            {
                throw QGitError("malloc", 0);
            }

            int c = 0;
            for(const auto &file: files)
            {
                pathspec.value.strings[c] = strdup(file.toUtf8().constData());
                c++;
            }
        }

        GitDiff diff;
        if (second == "staged")
        {
            GitObject first_obj;
            res = git_revparse_single(first_obj, repo, "HEAD^{tree}");
            if (res)
            {
                throw QGitError("git_revparse_single(staged)", res);
            }

            git_tree *first_tree = nullptr;
            res = git_tree_lookup(&first_tree, repo, git_object_id(first_obj));
            if (res)
            {
                throw QGitError("git_tree_lookup(staged)", res);
            }

            git_diff_options options;
            memset(&options, 0, sizeof(options));
            options.version = GIT_DIFF_OPTIONS_VERSION;
            options.flags = GIT_DIFF_INCLUDE_UNTRACKED | GIT_DIFF_SHOW_UNTRACKED_CONTENT;
            options.context_lines = context_lines;
            options.pathspec = pathspec;

            res = git_diff_tree_to_index(diff, repo, first_tree, nullptr, &options);
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
            options.context_lines = context_lines;
            options.pathspec = pathspec;

            res = git_diff_index_to_workdir(diff, repo, nullptr, &options);
            if (res)
            {
                throw QGitError("git_diff_index_to_workdir", res);
            }
        }
        else
        {
            GitObject second_obj;
            res = git_revparse_single(second_obj, repo, second.toUtf8());
            if (res)
            {
                throw QGitError("git_revparse_single(second)", res);
            }

            git_commit *second_commit = nullptr;
            res = git_commit_lookup(&second_commit, repo, git_object_id(second_obj));
            if (res)
            {
                throw QGitError("git_commit_lookup(second)", res);
            }

            git_tree *second_tree = nullptr;
            res = git_commit_tree(&second_tree, second_commit);
            if (res)
            {
                throw QGitError("git_commit_tree(second)", res);
            }

            res = git_diff_tree_to_tree(diff, repo, nullptr, second_tree, nullptr);
            if (res)
            {
                throw QGitError("git_diff_tree_to_tree", res);
            }
        }

        size_t _count = git_diff_num_deltas(diff);
        for(size_t c = 0; c < _count; c++)
        {
            const git_diff_delta *delta = git_diff_get_delta(diff, c);
            QGitDiffFile item(delta);

            if (files.contains(item.new_file().path()))
            {
                GitPatch patch;
                res = git_patch_from_diff(patch, diff, c);
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
                    size_t lines = git_patch_num_lines_in_hunk(patch, hunk_cnt);
                    if (lines > 0)
                    {
                        for(int  line_cnt = 0; line_cnt < lines; line_cnt++) {
                            const git_diff_line *line = nullptr;
                            res = git_patch_get_line_in_hunk(&line, patch, hunk_cnt, line_cnt);
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
    QGitError error;

    try
    {
        if (items.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
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
    QGitError error;

    try
    {
        if (items.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitStrArray paths;
        paths.value.count = static_cast<size_t>(items.count());
        paths.value.strings = static_cast<char **>(malloc(sizeof(char *) * paths.value.count));
        if (paths.value.strings == nullptr)
        {
            throw QGitError("malloc", 0);
        }

        for(int c = 0; c < items.count(); c++)
        {
            paths.value.strings[c] = strdup(items.at(c).toUtf8().constData());
        }

        GitReference head;
        GitObject head_commit;
        res = git_repository_head(head, repo);
        if (res == 0)
        {
            res = git_reference_peel(head_commit, head, GIT_OBJ_COMMIT);
            if (res)
            {
                throw QGitError("git_reference_peel", res);
            }
        }

        res = git_reset_default(repo, head_commit, &paths.value);
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
    QGitError error;

    try
    {
        if (lines.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        const git_index_entry *entry = git_index_get_bypath(index, filename.toUtf8().constData(), GIT_INDEX_STAGE_NORMAL);
        if (entry == nullptr)
        {
            throw QGitError("git_index_get_bypath", res);
        }

        GitBlob blob;
        res = git_blob_lookup(blob, repo, &entry->id);
        if (res)
        {
            throw QGitError("git_blob_lookup", res);
        }

        const char *blob_content = static_cast<const char *>(git_blob_rawcontent(blob));
        git_off_t blob_size = git_blob_rawsize(blob);
        QByteArray buffer = QByteArray(blob_content, blob_size);
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

    QGitError error;

    try
    {
        if (lines.count() == 0)
        {
            throw QGitError();
        }

        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        const git_index_entry *entry = nullptr;
        entry = git_index_get_bypath(index, filename.toUtf8().constData(), GIT_INDEX_STAGE_NORMAL);
        if (entry == nullptr)
        {
            throw QGitError("git_index_get_bypath", res);
        }

        GitBlob blob;
        res = git_blob_lookup(blob, repo, &entry->id);
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

void QGit::commit(QString message, bool withPush)
{
    git_oid new_commit_id = { {0} };
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        /// Check if somthing is staged
        GitTree tree;
        GitObject git_obj;
        res = git_revparse_single(git_obj, repo, "HEAD^{tree}");
        if (res == 0)
        {
            res = git_tree_lookup(tree, repo, git_object_id(git_obj));
            if (res)
            {
                throw QGitError("git_tree_lookup(staged)", res);
            }
        }

        GitDiff diff;
        res = git_diff_tree_to_index(diff, repo, tree, nullptr, nullptr);
        if (res)
        {
            throw QGitError("git_diff_tree_to_index(staged)", res);
        }

        int _count = git_diff_num_deltas(diff);
        if (_count == 0)
        {
            throw QGitError("Nothing staged.", res);
        }

        GitSignature me;
        res = git_signature_default(me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

        GitIndex index;
        res = git_repository_index(index, repo);
        if (res)
        {
            throw QGitError("git_repository_index", res);
        }

        git_oid tree_id = { {0} };
        res = git_index_write_tree(&tree_id, index);
        if (res)
        {
            throw QGitError("git_index_write_tree", res);
        }

        res = git_tree_lookup(tree, repo, &tree_id);
        if (res)
        {
            throw QGitError("git_tree_lookup", res);
        }

        int unborn = git_repository_head_unborn(repo);
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
            git_oid parent_id = { {0} };
            res = git_reference_name_to_id(&parent_id, repo, "HEAD");
            if (res)
            {
                throw QGitError("git_reference_name_to_id", res);
            }

            GitCommit parent;
            res = git_commit_lookup(parent, repo, &parent_id);
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
              parent.value);                 /* parent */
            if (res)
            {
                throw QGitError("git_commit_create_v", res);
            }
        }

        if (withPush)
        {

        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit commitReply(QString::fromUtf8(git_oid_tostr_s(&new_commit_id)), error);
}

void QGit::clone(QUrl url)
{
    QGitError error;

    try
    {
        git_clone_options opts;
        int res = git_clone_init_options(&opts, GIT_CLONE_OPTIONS_VERSION);
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

        GitRepository repo;
        res = git_clone(repo, url.toString().toUtf8().constData(), m_path.absolutePath().toUtf8().constData(), &opts);
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
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitRemote remote;
        res = git_remote_lookup(remote, repo, "origin");
        if (res)
        {
            throw QGitError("git_remote_lookup", res);
        }

        git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

        fetch_opts.callbacks.credentials = [](git_credential **out, const char *url, const char *username_from_url, unsigned int allowed_types, void *payload)
        {
            if (allowed_types & GIT_CREDENTIAL_SSH_KEY) {
                QString pass;
                QMetaObject::invokeMethod(QGitMasterMainWindow::instance(), &QGitMasterMainWindow::getPassword, Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, pass));

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

                        git_credential_ssh_key_new(out, username_from_url, pubkeyPathname, privkeyPathname, pass.toUtf8().constData());

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
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitStrArray remotes;
        res = git_remote_list(remotes, repo);
        if (res)
        {
            throw QGitError("git_remote_list", res);
        }

        QVector<QByteArray> remotesToFetch;

        if (fetchFromAllRemotes)
        {
            remotesToFetch.reserve(static_cast<int>(remotes.value.count));
            for(size_t idx = 0; idx < remotes.value.count; idx++)
            {
                remotesToFetch.append(QByteArray(remotes.value.strings[idx]));
            }
        }
        else
        {
            QByteArray selected;
            for(size_t idx = 0; idx < remotes.value.count; idx++)
            {
                if (QString::fromUtf8(remotes.value.strings[idx]) == QStringLiteral("origin"))
                {
                    selected = QByteArray(remotes.value.strings[idx]);
                    break;
                }
            }

            if (selected.isEmpty() && remotes.value.count > 0)
            {
                selected = QByteArray(remotes.value.strings[0]);
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
                QString pass;
                QMetaObject::invokeMethod(QGitMasterMainWindow::instance(), &QGitMasterMainWindow::getPassword, Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, pass));

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

                        git_credential_ssh_key_new(out, username_from_url, pubkeyPathname, privkeyPathname, pass.toUtf8().constData());

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
            GitRemote remote;
            res = git_remote_lookup(remote, repo, remoteName.constData());
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
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitRemote libgit2_remote;
        res = git_remote_lookup(libgit2_remote, repo, remote.toUtf8().constData());
        if (res)
        {
            throw QGitError("git_remote_lookup", res);
        }

        for(const auto &branch: branches)
        {
            GitReference ref;
            res = git_reference_lookup(ref, repo, branch.toUtf8().constData());
            if (res)
            {
                throw QGitError("Local branch not found!", res);
            }

            GitReference upstream_branch_ref;
            res = git_branch_upstream(upstream_branch_ref, ref);
            if (res)
            {
                throw QGitError("Local branch is not linked with remote branch!", res);
            }
        }

        GitStrArray refspecs;
        refspecs.value.count = branches.size();
        refspecs.value.strings = static_cast<char **>(malloc(sizeof(char *) * refspecs.value.count));
        for(int i = 0; i < branches.size(); i++)
        {
            QByteArray refspec = "refs/heads/" + branches[i].toUtf8() + ":refs/heads/" + branches[i].toUtf8();
            if (force) refspec.prepend("+");
            refspecs.value.strings[i] = strdup(refspec.constData());
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
                QString pass;
                QMetaObject::invokeMethod(QGitMasterMainWindow::instance(), &QGitMasterMainWindow::getPassword, Qt::BlockingQueuedConnection, Q_RETURN_ARG(QString, pass));

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

                        git_credential_ssh_key_new(out, username_from_url, pubkeyPathname, privkeyPathname, pass.toUtf8().constData());

                        return 0;
                    }
                }
            }

            return -1;
        };

        res = git_remote_push(libgit2_remote, &refspecs.value, &push_opts);
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
    QGitError error;

    try
    {
        GitRepository repo;
        int res = git_repository_open(repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        GitRevWalk walker;
        res = git_revwalk_new(walker, repo);
        if (res)
        {
            throw QGitError("git_revwalk_new", res);
        }

        git_oid oid;
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

        int count = 0;
        while ((!git_revwalk_next(&oid, walker))&&(count < length)) {

            QGitCommit item;
            unsigned int parents = 0;

            QString commit_id;
            QList<QGitCommitDiffParent> commit_parents;
            QDateTime commit_time;
            QGitSignature commit_author;
            QGitSignature commit_commiter;
            QString commit_message;

            GitCommit commit;
            res = git_commit_lookup(commit, repo, &oid);
            if (res)
            {
                throw QGitError("git_commit_lookup", res);
            }

            commit_id = QString::fromUtf8(git_oid_tostr_s(&oid));

            parents = git_commit_parentcount(commit);
            for(unsigned int index = 0; index < parents; index++)
            {
                GitCommit parent;
                QByteArray parentStr;

                res = git_commit_parent(parent, commit, index);
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
