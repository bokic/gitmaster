#include "qgit.h"
#include "git2.h"

#include "structs.h"
#include <QString>
#include <QDir>

QGit::QGit(QObject *parent)
    : QObject(parent)
{
    git_libgit2_init();
}

QGit::~QGit()
{
    git_libgit2_shutdown();
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
    git_repository *repo = NULL;
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

void QGit::repositoryStatus(QDir path)
{
    git_repository *repo = nullptr;
    git_status_list *list = nullptr;
    QMap<git_status_t, int> items;
    int result = 0;
    int index = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_status_list_new(&list, repo, nullptr);
    if (result)
    {
        emit error(__FUNCTION__, "git_status_list_new", result);
        goto cleanup;
    }

    while(const git_status_entry *item = git_status_byindex(list, index))
    {
        items[item->status]++;

        index++;
    }

    emit repositoryStatusReply(path, items);

cleanup:
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

void QGit::repositoryBranches(QDir path)
{
    QList<QGitBranch> branches;
    git_repository *repo = nullptr;
    git_branch_iterator *it = nullptr;
    git_reference *ref = nullptr;
    git_branch_t type = GIT_BRANCH_ALL;
    int result = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL);

    if (result)
    {
        emit error(__FUNCTION__, "git_branch_iterator_new", result);
        goto cleanup;
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

    emit repositoryBranchesReply(branches);

cleanup:
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

void QGit::repositoryStashes(QDir path)
{
    git_repository *repo = nullptr;
    QStringList stashes;
    int result = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_stash_foreach(repo, [](size_t index, const char* message, const git_oid *stash_id, void *payload) -> int {

            Q_UNUSED(index);
            Q_UNUSED(stash_id);

            QStringList *list = static_cast<QStringList *>(payload);

            list->append(message);

            return 0;
        }, &stashes);
    if (result)
    {
        emit error(__FUNCTION__, "git_stash_foreach", result);
        goto cleanup;
    }

    emit repositoryStashesReply(stashes);

cleanup:
    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::repositoryChangedFiles(QDir path)
{
    git_repository *repo = nullptr;
    git_status_list *list = nullptr;
    QMap<QString,git_status_t> items;
    int result = 0;
    int index = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_status_list_new(&list, repo, nullptr);
    if (result)
    {
        emit error(__FUNCTION__, "git_status_list_new", result);
        goto cleanup;
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
                emit error(__FUNCTION__, "unknown scenario", result);
                goto cleanup;
            }
        }

        index++;
    }

    emit repositoryChangedFilesReply(path, items);

cleanup:
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

void QGit::repositoryStageFiles(QDir path, QStringList items)
{
    git_repository *repo = nullptr;
    git_index *index = nullptr;
    int result = 0;

    if (items.count() == 0)
    {
        emit repositoryStageFilesReply(path);

        return;
    }

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_repository_index(&index, repo);
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_index", result);
        goto cleanup;
    }

    for(QString item: items)
    {
        result = git_index_add_bypath(index, item.toUtf8().constData());
        if (result)
        {
            emit error(__FUNCTION__, "git_index_add_bypath", result);
            goto cleanup;
        }
    }

    result = git_index_write(index);
    if (result)
    {
        emit error(__FUNCTION__, "git_index_write", result);
        goto cleanup;
    }

    emit repositoryStageFilesReply(path);

cleanup:
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

void QGit::repositoryUnstageFiles(QDir path, QStringList items)
{
    QList<QByteArray> tmpStrList;
    git_repository *repo = nullptr;
    git_reference *head = nullptr;
    git_object *head_commit = nullptr;
    git_strarray paths = {nullptr, 0};
    int result = 0;

    if (items.count() == 0)
    {
        emit repositoryUnstageFilesReply(path);

        return;
    }

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    paths.count = items.count();
    paths.strings = (char **)malloc(sizeof(char *) * path.count());

    for(int c = 0; c < items.count(); c++)
    {
        tmpStrList.append(items.at(c).toUtf8());
        paths.strings[c] = (char *)tmpStrList.at(c).data();
    }

    result = git_repository_head(&head, repo);
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_head", result);
        goto cleanup;
    }

    result = git_reference_peel(&head_commit, head, GIT_OBJ_COMMIT);
    if (result)
    {
        emit error(__FUNCTION__, "git_reference_peel", result);
        goto cleanup;
    }

    result = git_reset_default(repo, head_commit, &paths);
    if (result)
    {
        emit error(__FUNCTION__, "git_reset_default", result);
        goto cleanup;
    }

    emit repositoryUnstageFilesReply(path);

cleanup:
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

void QGit::repositoryCommit(QDir path, QString message)
{
    git_repository *repo = nullptr;
    git_signature *me = nullptr;
    git_commit *parent = nullptr;
    git_index *index = nullptr;
    git_tree *tree = nullptr;
    git_oid new_commit_id;
    git_oid parent_id;
    git_oid tree_id;
    int unborn = 0;
    int result = 0;

    memset(&new_commit_id, 0, sizeof(new_commit_id));
    memset(&parent_id, 0, sizeof(parent_id));
    memset(&tree_id, 0, sizeof(tree_id));

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_signature_default(&me, repo);
    if (result)
    {
        emit error(__FUNCTION__, "git_signature_default", result);
        goto cleanup;
    }

    result = git_repository_index(&index, repo);
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_index", result);
        goto cleanup;
    }

    result = git_index_write_tree(&tree_id, index);
    if (result)
    {
        emit error(__FUNCTION__, "git_index_write_tree", result);
        goto cleanup;
    }

    result = git_tree_lookup(&tree, repo, &tree_id);
    if (result)
    {
        emit error(__FUNCTION__, "git_tree_lookup", result);
        goto cleanup;
    }

    unborn = git_repository_head_unborn(repo);
    if (unborn)
    {
        result = git_commit_create_v(
          &new_commit_id,
          repo,
          "HEAD",                        /* name of ref to update */
          me,                            /* author */
          me,                            /* committer */
          nullptr,                       /* nullptr = UTF-8 message encoding */
          message.toUtf8().constData(),  /* message */
          tree,                          /* root tree */
          0);                            /* parent count */
    }
    else
    {
        result = git_reference_name_to_id(&parent_id, repo, "HEAD");
        if (result)
        {
            emit error(__FUNCTION__, "git_reference_name_to_id", result);
            goto cleanup;
        }

        result = git_commit_lookup(&parent, repo, &parent_id);
        if (result)
        {
            emit error(__FUNCTION__, "git_commit_lookup", result);
            goto cleanup;
        }

        result = git_commit_create_v(
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
    }

    if (result == 0)
    {
        emit repositoryCommitReply(path, QString::fromUtf8((const char *)new_commit_id.id));
    }
    else
    {
        emit error(__FUNCTION__, "git_commit_create", result);
    }

cleanup:
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

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::repositoryClone(QDir path, QUrl url)
{
    git_repository *repo = nullptr;
    git_clone_options opts;
#if LIBGIT2_SOVERSION > 22
#else
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
#endif
    int result = 0;

    result = git_clone_init_options(&opts, GIT_CLONE_OPTIONS_VERSION);
    if (result)
    {
        goto cleanup;
    }

#if LIBGIT2_SOVERSION > 22
    opts.fetch_opts.callbacks.payload = this;
    opts.fetch_opts.callbacks.transfer_progress = [](const git_transfer_progress *stats, void *payload) -> int {

        QGit *_this =  static_cast<QGit *>(payload);

        emit _this->repositoryCloneTransferReply(stats->total_objects, stats->indexed_objects, stats->received_objects, stats->local_objects, stats->total_deltas, stats->indexed_deltas, stats->received_bytes);

        return _this->m_abort;
    };
#else
    callbacks.payload = this;
    callbacks.transfer_progress = [](const git_transfer_progress *stats, void *payload) -> int {

        QGit *_this =  static_cast<QGit *>(payload);

        emit _this->repositoryCloneTransferReply(stats->total_objects, stats->indexed_objects, stats->received_objects, stats->local_objects, stats->total_deltas, stats->indexed_deltas, stats->received_bytes);

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
        QGit *_this = static_cast<QGit *>(payload);

        emit _this->repositoryCloneProgressReply(QString::fromUtf8(path), completed_steps, total_steps);

        _this = nullptr;
    };

    m_abort = 0;

    result = git_clone(&repo, url.toString().toUtf8().constData(), path.absolutePath().toUtf8().constData(), &opts);

cleanup:
    emit repositoryCloneReply(path, result);

    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}

void QGit::repositoryPull(QDir path)
{
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;
    int result = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_remote_lookup(&remote, repo, "origin");
    if (result)
    {
        emit error(__FUNCTION__, "git_remote_lookup", result);
        goto cleanup;
    }

    result = git_remote_fetch(remote, nullptr, nullptr, "pull");
    if (result)
    {
        emit error(__FUNCTION__, "git_remote_fetch", result);
        goto cleanup;
    }

    emit repositoryPull(path);

cleanup:
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

void QGit::repositoryFetch(QDir path)
{
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;
    int result = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_remote_lookup(&remote, repo, "origin");
    if (result)
    {
        emit error(__FUNCTION__, "git_remote_lookup", result);
        goto cleanup;
    }

    result = git_remote_fetch(remote, nullptr, nullptr, "fetch");
    if (result)
    {
        emit error(__FUNCTION__, "git_remote_fetch", result);
        goto cleanup;
    }

    emit repositoryFetch(path);

cleanup:
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

void QGit::repositoryPush(QDir path)
{
    git_repository *repo = nullptr;
    git_remote *remote = nullptr;
    int result = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());
    if (result)
    {
        emit error(__FUNCTION__, "git_repository_open", result);
        return;
    }

    result = git_remote_lookup(&remote, repo, "origin");
    if (result)
    {
        emit error(__FUNCTION__, "git_remote_lookup", result);
        goto cleanup;
    }

#if LIBGIT2_SOVERSION > 22
    result = git_remote_push(remote, nullptr, nullptr);
#else
    result = git_remote_push(remote, nullptr, nullptr, nullptr, "push");
#endif
    if (result)
    {
        emit error(__FUNCTION__, "git_remote_fetch", result);
        goto cleanup;
    }

    emit repositoryPushReply(path);

cleanup:
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
