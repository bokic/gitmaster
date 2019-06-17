#include "qgit.h"
#include "qgitdifffile.h"
#include "qgitdiffhunk.h"
#include "qgitdiffline.h"
#include "qgiterror.h"
#include "git2.h"

#include <QWriteLocker>
#include <QReadLocker>
#include <QString>
#include <QVector>
#include <QDir>


QGit::QGit(const QDir &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
{
    git_libgit2_init();
}

QGit::QGit(QObject *parent)
    : QObject(parent)
{
    git_libgit2_init();
}

QGit::~QGit()
{
    git_libgit2_shutdown();
}

bool QGit::setPath(const QDir &path)
{
    if (!m_pathLock.tryLockForWrite())
    {
        return false;
    }

    m_path = path;

    m_pathLock.unlock();

    return true;
}

QDir QGit::path() const
{
    return m_path;
}

void QGit::abort()
{
    m_abort = 1;
}

QString QGit::getBranchNameFromPath(const QString &path)
{
    git_repository *repo = nullptr;
    git_reference *ref = nullptr;
    int result = 0;
    const char *branch = nullptr;
    QString ret;

    git_libgit2_init();

    result = git_repository_open(&repo, path.toUtf8().constData());
    if (result)
    {
        goto cleanup;
    }

    result = git_repository_head(&ref, repo);
    if (result)
    {
        goto cleanup;
    }

    git_branch_name(&branch, ref);
    ret = branch;

cleanup:
    if (ref)
    {
        git_reference_free(ref);
        ref = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }

    git_libgit2_shutdown();

    return ret;
}

int QGit::createLocalRepository(const QDir &path)
{
    git_repository *repo = nullptr;
    int err = 0;

    git_libgit2_init();

    err = git_repository_init(&repo, path.absolutePath().toUtf8().constData(), 0);

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }

    git_libgit2_shutdown();

    return err;
}

bool QGit::isGitRepository(const QDir &path)
{
    git_repository *repo = nullptr;
    bool ret = false;
    int err = 0;

    git_libgit2_init();

    err = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (err == 0)
    {
        ret = true;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }

    git_libgit2_shutdown();

    return ret;
}

bool QGit::gitRepositoryDefaultSignature(const QDir &path, QString &name, QString &email)
{
    git_repository *repo = nullptr;
    git_signature *me = nullptr;
    int result = 0;
    bool ret = false;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        goto cleanup;
    }

    result = git_signature_default(&me, repo);
    if (result)
    {
        goto cleanup;
    }

    name = me->name;
    email = me->email;

    ret = true;

cleanup:
    if (me)
    {
        git_signature_free(me);
        me = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }

    return ret;
}

void QGit::currentBranch()
{
    git_repository *repo = nullptr;
    git_reference *ref = nullptr;
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

    if (ref)
    {
        git_reference_free(ref);
        ref = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::init()
{
    git_repository *repo = nullptr;
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

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::signature()
{
    git_repository *repo = nullptr;
    git_signature *me = nullptr;
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

    if (me)
    {
        git_signature_free(me);
        me = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::status()
{
    git_repository *repo = nullptr;
    git_status_list *list = nullptr;
    QMap<git_status_t, int> items;
    size_t index = 0;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
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

    if (list)
    {
        git_status_list_free(list);
        list = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::listBranchesAndTags()
{
    QList<QGitBranch> branches;
    git_repository *repo = nullptr;
    git_branch_iterator *it = nullptr;
    git_reference *ref = nullptr;
    git_branch_t type = GIT_BRANCH_ALL;
    git_strarray tag_names = {nullptr, 0};
    QList<QString> tags;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL);
        if (res)
        {
            throw QGitError("git_branch_iterator_new", res);
        }

        while(git_branch_next(&ref, &type, it) == 0)
        {
            const char *ref_name = git_reference_name(ref);

            QGitBranch branch = QGitBranch(ref_name, type);
            branches.append(branch);

            ref_name = nullptr;
            git_reference_free(ref);
            ref = nullptr;
        }

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

    if (tag_names.strings)
    {
        git_strarray_free(&tag_names);
        tag_names = {nullptr, 0};
    }

    if(ref)
    {
        git_reference_free(ref);
        ref = nullptr;
    }

    if (it)
    {
        git_branch_iterator_free(it);
        it = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::stashSave(QString name)
{
    git_repository *repo = nullptr;
    git_reference *head = nullptr;
    //git_commit *commit = nullptr;
    git_signature *me = nullptr;
    //git_index *index = nullptr;
    git_oid *oid = nullptr;

    QStringList stashes;
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

        res = git_repository_head(&head, repo);
        if (res)
        {
            throw QGitError("git_repository_head", res);
        }

        // TODO: Check if we have memory leak here!
        oid = const_cast<git_oid *>(git_reference_target(head));
        if (!oid)
        {
            throw QGitError("git_reference_target returned NULL", 0);
        }

        res = git_stash_save(oid, repo, me, name.toUtf8().constData(), 0);
        if (res)
        {
            throw QGitError("git_stash_save", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit stashSaveReply(error);

    if (oid)
    {
        // TODO: Check if we need to free memory here!
        //git_oid_shorten_free();
        oid = nullptr;
    }

    if (head)
    {
        git_reference_delete(head);
        head = nullptr;
    }

    if (me)
    {
        git_signature_free(me);
        me = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::stashRemove(QString name)
{
    Q_UNUSED(name)
}

void QGit::listStashes()
{
    git_repository *repo = nullptr;
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

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::listChangedFiles()
{
    git_repository *repo = nullptr;
    git_status_list *list = nullptr;
    QMap<QString,git_status_t> items;
    int res = 0;
    size_t index = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
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
            git_status_t status = item->status;
            if ((status != GIT_STATUS_CURRENT)&&(status != GIT_STATUS_IGNORED))
            {
                if (item->index_to_workdir)
                {
                    items.insert(QString::fromUtf8(item->index_to_workdir->new_file.path), status);
                }
                else if (item->head_to_index)
                {
                    items.insert(QString::fromUtf8(item->head_to_index->new_file.path), status);
                }
                else
                {
                    throw QGitError("unknown scenario", res);
                }
            }

            index++;
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listChangedFilesReply(items, error);

    if (list)
    {
        git_status_list_free(list);
        list = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::commitDiff(QString commitId)
{
    const git_diff_delta *delta = nullptr;
    git_tree *commit_tree = nullptr;
    git_tree *parent_tree = nullptr;
    git_repository *repo = nullptr;
    git_commit *parent = nullptr;
    git_commit *commit = nullptr;
    git_object *obj = nullptr;
    git_diff *diff = nullptr;
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

        res = git_revparse_single(&obj, repo, commitId.toLatin1());
        if (res)
        {
            throw QGitError("git_revparse_single", res);
        }

        res = git_commit_lookup(&commit, repo, git_object_id(obj));
        if (res)
        {
            throw QGitError("git_commit_lookup", res);
        }

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
        commitTime.setOffsetFromUtc(timeOffset * 60);

        commitMessage = QString::fromUtf8(git_commit_message(commit));

        parentCount = git_commit_parentcount(commit);

        if (parentCount == 0)
        {
            QGitCommitDiffParent item;

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

            git_diff_free(diff);
            diff = nullptr;

            parents.append(item);
        }
        else
        {
            for(unsigned int c = 0; c < parentCount; c++)
            {
                res = git_commit_parent(&parent, commit, c);
                if (res)
                {
                    throw QGitError("git_commit_parent", res);
                }

                char commit_id[41] = {0, };
                const git_oid *oid = git_commit_id(parent);
                git_oid_tostr(commit_id, 41, oid);
                QGitCommitDiffParent item(commit_id);

                res = git_commit_tree(&parent_tree, parent);
                if (res)
                {
                    throw QGitError("git_commit_tree", res);
                }

                res = git_diff_tree_to_tree(&diff, repo, parent_tree, commit_tree, nullptr);
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

                git_diff_free(diff);
                diff = nullptr;

                git_tree_free(parent_tree);
                parent_tree = nullptr;

                git_commit_free(parent);
                parent = nullptr;

                parents.append(item);
            }
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    commitDiff = QGitCommit(commitId, parents, commitTime, commitAuthor, commitCommiter, commitMessage);

    emit commitDiffReply(commitId, commitDiff, error);

    if (diff)
    {
        git_diff_free(diff);
        diff = nullptr;
    }

    if (parent_tree)
    {
        git_tree_free(parent_tree);
        parent_tree = nullptr;
    }

    if (parent)
    {
        git_commit_free(parent);
        parent = nullptr;
    }

    if (commit_tree)
    {
        git_tree_free(commit_tree);
        commit_tree = nullptr;
    }

    if(commit)
    {
        git_commit_free(commit);
        commit = nullptr;
    }

    if(obj)
    {
        git_object_free(obj);
        obj = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::commitDiffContent(QString first, QString second, QList<QString> files)
{
    git_commit *first_commit = nullptr, *second_commit = nullptr;
    git_object *first_obj = nullptr, *second_obj = nullptr;
    git_tree *first_tree = nullptr, *second_tree = nullptr;
    const git_diff_delta *delta = nullptr;
    git_repository *repo = nullptr;
    git_patch *patch = nullptr;
    QList<QGitDiffFile> items;
    git_diff *diff = nullptr;
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
            res = git_revparse_single(&first_obj, repo, first.toLatin1());
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

            res = git_diff_index_to_workdir(&diff, repo, nullptr, &options);
            if (res)
            {
                throw QGitError("git_diff_index_to_workdir", res);
            }
        }
        else
        {
            res = git_revparse_single(&second_obj, repo, second.toLatin1());
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

                git_patch_free(patch);
            }
        }
    } catch(const QGitError &ex) {
        error = ex;
    }

    emit commitDiffContentReply(first, second, items, error);

    if (diff)
    {
        git_diff_free(diff);
        diff = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::stageFiles(QStringList items)
{
    git_repository *repo = nullptr;
    git_index *index = nullptr;
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

    if (index)
    {
        git_index_free(index);
        index = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::unstageFiles(QStringList items)
{
    QVector<QByteArray> tmpStrList;
    git_repository *repo = nullptr;
    git_reference *head = nullptr;
    git_object *head_commit = nullptr;
    git_strarray paths = {nullptr, 0};
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
        paths.strings = static_cast<char **>(malloc(sizeof(char *) * static_cast<size_t>(items.count())));
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

    if (paths.strings)
    {
        free(paths.strings);
        paths.strings = nullptr;
    }
    paths.count = 0;

    if (head_commit)
    {
        git_object_free(head_commit);
        head_commit = nullptr;
    }

    if (head)
    {
        git_reference_free(head);
        head = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::commit(QString message)
{
    git_repository *repo = nullptr;
    git_signature *me = nullptr;
    git_commit *parent = nullptr;
    git_index *index = nullptr;
    git_tree *tree = nullptr;
    git_oid new_commit_id = { {0} };
    git_oid parent_id = { {0} };
    git_oid tree_id = { {0} };
    git_object *git_obj = nullptr;
    git_diff *diff = nullptr;
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
        res = git_revparse_single(&git_obj, repo, "HEAD^{tree}");
        if (res)
        {
            throw QGitError("git_revparse_single(staged)", res);
        }

        res = git_tree_lookup(&tree, repo, git_object_id(git_obj));
        if (res)
        {
            throw QGitError("git_tree_lookup(staged)", res);
        }

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

        if (tree)
        {
            git_tree_free(tree);
            tree = nullptr;
        }
        /// Check end

        res = git_signature_default(&me, repo);
        if (res)
        {
            throw QGitError("git_signature_default", res);
        }

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

    emit commitReply(QString::fromUtf8(reinterpret_cast<const char *>(new_commit_id.id)), error);

    if (parent)
    {
        git_commit_free(parent);
        parent = nullptr;
    }


    if (index)
    {
        git_index_free(index);
        index = nullptr;
    }

    if (tree)
    {
        git_tree_free(tree);
        tree = nullptr;
    }

    if (me)
    {
        git_signature_free(me);
        me = nullptr;
    }

    if (diff)
    {
        git_diff_free(diff);
        diff = nullptr;
    }

    if (git_obj)
    {
        git_object_free(git_obj);
        git_obj = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::clone(QUrl url)
{
    git_repository *repo = nullptr;
    git_clone_options opts;
#if LIBGIT2_SOVERSION <= 22
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
#endif
    int res = 0;

    QGitError error;

    try {

        res = git_clone_init_options(&opts, GIT_CLONE_OPTIONS_VERSION);
        if (res)
        {
            throw QGitError("git_clone_init_options", res);
        }

#if LIBGIT2_SOVERSION > 22
        opts.fetch_opts.callbacks.payload = this;
        opts.fetch_opts.callbacks.transfer_progress = [](const git_transfer_progress *stats, void *payload) -> int {

            auto _this = static_cast<QGit *>(payload);

            emit _this->cloneTransferReply(stats->total_objects, stats->indexed_objects, stats->received_objects, stats->local_objects, stats->total_deltas, stats->indexed_deltas, stats->received_bytes);

            return _this->m_abort;
        };
#else
        callbacks.payload = this;
        callbacks.transfer_progress = [](const git_transfer_progress *stats, void *payload) -> int {

            QGit *_this =  static_cast<QGit *>(payload);

			emit _this->cloneTransferReply(stats->total_objects, stats->indexed_objects, stats->received_objects, stats->local_objects, stats->total_deltas, stats->indexed_deltas, stats->received_bytes);

            return _this->m_abort;
        };

        opts.remote_callbacks = callbacks;
#endif
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

        m_abort = 0;

        res = git_clone(&repo, url.toString().toUtf8().constData(), m_path.absolutePath().toUtf8().constData(), &opts);
        if (res)
        {
            throw QGitError("git_clone", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit cloneReply(error);

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::pull()
{
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_remote_lookup(&remote, repo, "origin");
        if (res)
        {
            throw QGitError("git_remote_lookup", res);
        }

        res = git_remote_fetch(remote, nullptr, nullptr, "pull");
        if (res)
        {
            throw QGitError("git_remote_fetch", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit pullReply(error);

    if (remote)
    {
        git_remote_free(remote);
        remote = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::fetch()
{
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_remote_lookup(&remote, repo, "origin");
        if (res)
        {
            throw QGitError("git_remote_lookup", res);
        }

        res = git_remote_fetch(remote, nullptr, nullptr, "fetch");
        if (res)
        {
            throw QGitError("git_remote_fetch", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit fetchReply(error);

    if (remote)
    {
        git_remote_free(remote);
        remote = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::push()
{
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

        res = git_remote_lookup(&remote, repo, "origin");
        if (res)
        {
            throw QGitError("git_remote_lookup", res);
        }

#if LIBGIT2_SOVERSION > 22
        res = git_remote_push(remote, nullptr, nullptr);
#else
        res = git_remote_push(remote, nullptr, nullptr, nullptr, "push");
#endif
        if (res)
        {
            throw QGitError("git_remote_push", res);
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit pushReply(error);

    if (remote)
    {
        git_remote_free(remote);
        remote = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::listCommits(QString object, int length)
{
    QList<QGitCommit> commits;
    git_repository *repo = nullptr;
    git_revwalk *walker = nullptr;
    git_commit *parent = nullptr;
    git_commit *commit = nullptr;
    git_diff *diff = nullptr;
    git_oid oid;
    int count = 0;
    int res = 0;

    QGitError error;

    try {

        res = git_repository_open(&repo, m_path.absolutePath().toUtf8().constData());
        if (res)
        {
            throw QGitError("git_repository_open", res);
        }

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
            res = git_oid_fromstr(&oid, object.toLatin1().constData());
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

            res = git_commit_lookup(&commit, repo, &oid);
            if (res)
            {
                throw QGitError("git_commit_lookup", res);
            }

            commit_id = QString::fromUtf8(git_oid_tostr_s(&oid));

            parents = git_commit_parentcount(commit);
            for(unsigned int index = 0; index < parents; index++)
            {
                git_commit *parent = nullptr;
                QByteArray parentStr;

                res = git_commit_parent(&parent, commit, index);
                if (res)
                {
                    throw QGitError("git_commit_parent", res);
                }

                const git_oid *parent_iod =  git_commit_id(parent);
                parentStr = QByteArray(git_oid_tostr_s(parent_iod));

                commit_parents.append(QGitCommitDiffParent(parentStr));

                git_commit_free(parent);
                parent = nullptr;
            }

            auto time = git_commit_time(commit);
            auto timeOffset = git_commit_time_offset(commit);
            commit_time = QDateTime::fromMSecsSinceEpoch(time * 1000);
            commit_time.setOffsetFromUtc(timeOffset * 60);

            QString author_name = QString::fromUtf8(git_commit_author(commit)->name);
            QString author_email = QString::fromUtf8(git_commit_author(commit)->email);
            QDateTime author_when = QDateTime::fromMSecsSinceEpoch(git_commit_author(commit)->when.time * 1000);
            author_when.setOffsetFromUtc(git_commit_author(commit)->when.offset * 60);
            commit_author = QGitSignature(author_name, author_email, author_when);

            QString commiter_name =  QString::fromUtf8(git_commit_committer(commit)->name);
            QString commiter_email = QString::fromUtf8(git_commit_committer(commit)->email);
            QDateTime commiter_when = QDateTime::fromMSecsSinceEpoch(git_commit_committer(commit)->when.time * 1000);
            commiter_when.setOffsetFromUtc(git_commit_committer(commit)->when.offset * 60);
            commit_commiter = QGitSignature(commiter_name, commiter_email, commiter_when);

            commit_message = QString::fromUtf8(git_commit_message(commit));

            item = QGitCommit(commit_id, commit_parents, commit_time, commit_author, commit_commiter, commit_message);

            commits.push_back(item);

            git_diff_free(diff);
            diff = nullptr;

            git_commit_free(parent);
            parent = nullptr;

            git_commit_free(commit);
            commit = nullptr;

            count++;
        }

    } catch(const QGitError &ex) {
        error = ex;
    }

    emit listCommitsReply(commits, error);

    if (diff)
    {
        git_diff_free(diff);
        diff = nullptr;
    }

    if (parent)
    {
        git_commit_free(parent);
        parent = nullptr;
    }

    if (walker)
    {
        git_revwalk_free(walker);
        walker = nullptr;
    }

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}
