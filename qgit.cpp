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
        goto exit1;
    }

    result = git_repository_head(&ref, repo);
    if (result)
    {
        goto exit2;
    }

    git_branch_name(&branch, ref);
    ret = branch;

    git_reference_free(ref);
    ref = nullptr;

exit2:
    git_repository_free(repo);
    repo = nullptr;

exit1:
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
        goto exit1;
    }

    while(const git_status_entry *item = git_status_byindex(list, index))
    {
        items[item->status]++;

        index++;
    }

    emit repositoryStatusReply(path, items);

    git_status_list_free(list);
    list = nullptr;

exit1:
    git_repository_free(repo);
    repo = nullptr;
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
        goto exit1;
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

    git_branch_iterator_free(it);
    it = nullptr;

exit1:

    git_repository_free(repo);
    repo = nullptr;
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
        goto exit1;
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
            }
        }

        index++;
    }

    emit repositoryChangedFilesReply(path, items);

    git_status_list_free(list);
    list = nullptr;

exit1:
    git_repository_free(repo);
    repo = nullptr;
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
        goto exit1;
    }

    for(QString item: items)
    {
        result = git_index_add_bypath(index, item.toUtf8().constData());
        if (result)
        {
            emit error(__FUNCTION__, "git_index_add_bypath", result);
            goto exit1;
        }
    }

    result = git_index_write(index);
    if (result)
    {
        emit error(__FUNCTION__, "git_index_write", result);
        goto exit2;
    }

    emit repositoryStageFilesReply(path);

exit2:
    git_index_free(index);
    index = nullptr;

exit1:
    git_repository_free(repo);
    repo = nullptr;
}

void QGit::repositoryUnstageFiles(QDir path, QStringList items)
{
    QList<QByteArray> tmpStrList;
    git_repository *repo = nullptr;
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

    result = git_reset_default(repo, nullptr, &paths);
    if (result)
    {
        emit error(__FUNCTION__, "git_reset_default", result);
        goto exit1;
    }

    emit repositoryUnstageFilesReply(path);

exit1:
    free(paths.strings);
    paths.strings = nullptr;
    paths.count = 0;

    git_repository_free(repo);
    repo = nullptr;
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
        goto exit1;
    }

    result = git_repository_index(&index, repo);
    result = git_index_write_tree(&tree_id, index);
    result = git_tree_lookup(&tree, repo, &tree_id);

    unborn = git_repository_head_unborn(repo);
    if (unborn)
    {
        git_index_free(index);
        index = nullptr;

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
        result = git_commit_lookup(&parent, repo, &parent_id);

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

    git_commit_free(parent);
    parent = nullptr;

exit3:
    git_tree_free(tree);
    tree = nullptr;

exit2:
    git_signature_free(me);
    me = nullptr;

exit1:
    git_repository_free(repo);
    repo = nullptr;
}

void QGit::repositoryClone(QDir path, QUrl url)
{
    git_repository *repo = nullptr;
    git_clone_options opts;
    int result = 0;

    result = git_clone_init_options(&opts, GIT_CLONE_OPTIONS_VERSION);
    if (result)
    {
        emit error(__FUNCTION__, "git_clone_init_options", result);

        goto exit;
    }

    opts.checkout_opts.progress_payload = this;
    opts.checkout_opts.progress_cb = [](
            const char *path,
            size_t completed_steps,
            size_t total_steps,
            void *payload)
    {
        QGit *_this = (QGit *)payload;

        emit _this->repositoryCloneProgressReply(QString::fromUtf8(path), completed_steps, total_steps);

        _this = nullptr;
    };

    result = git_clone(&repo, url.toString().toUtf8().constData(), path.absolutePath().toUtf8().constData(), &opts);

    if (result)
    {
        emit error(__FUNCTION__, "git_clone", result);
    }
    else
    {
        emit repositoryCloneReply(path);
    }

exit:
    if (repo)
    {
        git_repository_free(repo);
        repo = nullptr;
    }
}
