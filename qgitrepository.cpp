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

    connect(this, SIGNAL(repositoryStageFiles(QDir,QStringList)), m_git, SLOT(repositoryStageFiles(QDir,QStringList)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(repositoryStageFilesReply(QDir)), this, SLOT(repositoryStageFilesReply(QDir)), Qt::QueuedConnection);

    connect(this, SIGNAL(repositoryUnstageFiles(QDir,QStringList)), m_git, SLOT(repositoryUnstageFiles(QDir,QStringList)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(repositoryUnstageFilesReply(QDir)), this, SLOT(repositoryUnstageFilesReply(QDir)), Qt::QueuedConnection);

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
        int tmp_status = GIT_STATUS_CURRENT;

#if ((LIBGIT2_VER_MAJOR > 0)||(LIBGIT2_VER_MINOR >= 23))
        if (status & GIT_STATUS_CONFLICTED)
        {
            continue;
        }
#endif

        tmp_status = status & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED | GIT_STATUS_INDEX_DELETED | GIT_STATUS_INDEX_RENAMED | GIT_STATUS_INDEX_TYPECHANGE);
        if (tmp_status)
        {
            QListWidgetItem *item = new QListWidgetItem(file);

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked);

            switch(tmp_status)
            {
            case GIT_STATUS_INDEX_NEW:
                item->setIcon(QIcon(":/images/file_new.svg"));
                break;
            case GIT_STATUS_INDEX_MODIFIED:
                item->setIcon(QIcon(":/images/file_modified.svg"));
                break;
            case GIT_STATUS_INDEX_DELETED:
                item->setIcon(QIcon(":/images/file_removed.svg"));
                break;
            default:
                // TODO: Add some "unknown" image.
                break;
            }

            ui->listWidget_staged->addItem(item);
        }

        tmp_status = status & (GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_UNREADABLE);
        if (tmp_status)
        {
            QListWidgetItem *item = new QListWidgetItem(file);

            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Unchecked);

            switch(tmp_status)
            {
            case GIT_STATUS_WT_NEW:
                item->setIcon(QIcon(":/images/file_new.svg"));
                break;
            case GIT_STATUS_WT_MODIFIED:
                item->setIcon(QIcon(":/images/file_modified.svg"));
                break;
            case GIT_STATUS_WT_DELETED:
                item->setIcon(QIcon(":/images/file_removed.svg"));
                break;
            default:
                // TODO: Add some "unknown" image.
                break;
            }

            ui->listWidget_unstaged->addItem(item);
        }
    }
}

void QGitRepository::repositoryStageFilesReply(QDir path)
{
    emit repositoryChangedFiles(m_path);
}

void QGitRepository::repositoryUnstageFilesReply(QDir path)
{
    emit repositoryChangedFiles(m_path);
}

void QGitRepository::on_repositoryDetail_currentChanged(int index)
{
    switch(index) {
    case 0:
        emit repositoryChangedFiles(m_path);
        break;
    case 1:
        Q_UNIMPLEMENTED();
        break;
    case 2:
        Q_UNIMPLEMENTED();
        break;
    default:
        Q_UNIMPLEMENTED();
        break;
    }
}

void QGitRepository::on_checkBox_StagedFiles_clicked()
{
    QStringList items;

    ui->checkBox_StagedFiles->setChecked(true);

    for(int c = 0; c < ui->listWidget_staged->count(); c++)
    {
        QString file = ui->listWidget_staged->item(c)->text();

        items.append(file);
    }

    if (items.count() > 0)
    {
        emit repositoryUnstageFiles(m_path, items);
    }
}

void QGitRepository::on_checkBox_UnstagedFiles_clicked()
{
    QStringList items;

    ui->checkBox_UnstagedFiles->setChecked(false);

    for(int c = 0; c < ui->listWidget_unstaged->count(); c++)
    {
        QString file = ui->listWidget_unstaged->item(c)->text();

        items.append(file);
    }

    if (items.count() > 0)
    {
        emit repositoryStageFiles(m_path, items);
    }
}

void QGitRepository::on_listWidget_staged_itemChanged(QListWidgetItem *item)
{
    if (item->checkState() == Qt::Unchecked)
    {
        QStringList items;

        items.append(item->text());

        emit repositoryUnstageFiles(m_path, items);

        ui->listWidget_staged->takeItem(ui->listWidget_staged->row(item));
    }
}

void QGitRepository::on_listWidget_unstaged_itemChanged(QListWidgetItem *item)
{
    if (item->checkState() == Qt::Checked)
    {
        QStringList items;

        items.append(item->text());

        emit repositoryStageFiles(m_path, items);

        ui->listWidget_unstaged->takeItem(ui->listWidget_unstaged->row(item));
    }
}
