#include "qgit.h"
#include "git2.h"

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

void QGit::repositoryStatus(QDir path)
{
    git_repository *repo = nullptr;
    git_status_list *list = nullptr;
    QHash<git_status_t, int> items;
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
