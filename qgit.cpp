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

void QGit::statusRepository(const QDir &path)
{
    git_repository *repo = nullptr;
    git_status_list *list = nullptr;
    int result = 0;

    result = git_repository_open(&repo, path.absolutePath().toUtf8().constData());

    result = git_status_list_new(&list, repo, nullptr);

    const git_status_entry *item = git_status_byindex(list, 0);

    git_repository_free(repo);
    repo = nullptr;
}
