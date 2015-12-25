#include "qgitrepository.h"
#include "ui_qgitrepository.h"
#include <QTreeWidgetItem>
#include <QDebug>
#include <QList>

QGitRepository::QGitRepository(const QString &path, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QGitRepository)
    , m_path(path)
    , m_git(new QGit(this))
{
    ui->setupUi(this);

    connect(this, SIGNAL(repositoryBranches(QDir)), m_git, SLOT(repositoryBranches(QDir)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(repositoryBranchesReply(QList<QGitBranch>)), this, SLOT(repositoryBranchesReply(QList<QGitBranch>)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryChangedFiles(QDir)), m_git, SLOT(repositoryChangedFiles(QDir)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(repositoryChangedFilesReply(QDir,QMap<QString,git_status_t>)), this, SLOT(repositoryChangedFilesReply(QDir,QMap<QString,git_status_t>)), Qt::QueuedConnection);

    on_repositoryDetail_currentChanged(ui->repositoryDetail->currentIndex());

    emit repositoryBranches(QDir(m_path));
}

QGitRepository::~QGitRepository()
{
    delete ui;
}

void QGitRepository::repositoryBranchesReply(QList<QGitBranch> branches)
{
    QList<QTreeWidgetItem *> items;
    QTreeWidgetItem *item_file_status = new QTreeWidgetItem(QStringList() << tr("File Status"));
    QTreeWidgetItem *item_branches = new QTreeWidgetItem(QStringList() << tr("Branches"));
    QTreeWidgetItem *item_tags = new QTreeWidgetItem(QStringList() << tr("Tags"));
    QTreeWidgetItem *item_remotes = new QTreeWidgetItem(QStringList() << tr("Remotes"));

    ui->branchesTreeView->clear();

    item_file_status->addChild(new QTreeWidgetItem(QStringList() << tr("Working Copy")));

    for(auto branch: branches)
    {
        QStringList items = branch.name.split('/');
        QTreeWidgetItem *branch_item = nullptr;
        QString originStr, branchStr;

        switch(branch.type)
        {
        case GIT_BRANCH_LOCAL:
            if (items.count() == 3)
            {
                branchStr = items.at(2);
                branch_item = new QTreeWidgetItem(QStringList() << branchStr);
                item_branches->addChild(branch_item);
            }
            else
            {
                qDebug() << "Invalid local branch item.";
            }
            break;
        case GIT_BRANCH_REMOTE:
            if (items.count() == 4)
            {
                originStr = items.at(2);
                branchStr = items.at(3);

                for(int c = 0; c < item_remotes->childCount(); c++)
                {
                    if (item_remotes->child(c)->text(0) == originStr)
                    {
                        branch_item = item_remotes->child(c);

                        break;
                    }
                }

                if (branch_item == nullptr)
                {
                    branch_item = new QTreeWidgetItem(QStringList() << originStr);
                }

                branch_item->addChild(new QTreeWidgetItem(QStringList() << branchStr));

                item_remotes->addChild(branch_item);
            }
            else
            {
                qDebug() << "Invalid remote branch item";
            }
            break;
        case GIT_BRANCH_ALL:
            // TODO: Implement QGitRepository::repositoryBranchesReply - GIT_BRANCH_ALL
            qDebug() << "Unimplemented(GIT_BRANCH_ALL)";
            break;
        }
    }

    items.append(item_file_status);
    items.append(item_branches);
    items.append(item_tags); // TODO: Implement git tags.
    items.append(item_remotes);

    ui->branchesTreeView->addTopLevelItems(items);

    ui->branchesTreeView->expandAll();
}

void QGitRepository::repositoryChangedFilesReply(QDir path, QMap<QString, git_status_t> files)
{
    Q_UNUSED(path);

    ui->listWidget_staged->clear();
    ui->listWidget_unstaged->clear();

    for(auto file: files.keys())
    {
        const git_status_t status = files.value(file);

#if ((LIBGIT2_VER_MAJOR > 0)||(LIBGIT2_VER_MINOR >= 23))
        if (status & GIT_STATUS_CONFLICTED)
        {
            continue;
        }
#endif

        if (status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE))
        {
            ui->listWidget_staged->addItem(file);
        }

        if (status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_UNREADABLE))
        {
            ui->listWidget_unstaged->addItem(file);
        }
    }
}

void QGitRepository::on_repositoryDetail_currentChanged(int index)
{
    switch(index) {
    case 0:
        emit repositoryChangedFiles(m_path);
        break;
    case 1:
        break;
    case 2:
        break;
    default:
        break;
    }

    Q_UNIMPLEMENTED();
}
