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
    git_repository *repo = NULL;
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
