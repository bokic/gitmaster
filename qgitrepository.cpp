#include "qgitrepository.h"
#include "ui_qgitrepository.h"
#include <QTreeWidgetItem>
#include <QDebug>
#include <QList>

QGitRepository::QGitRepository(const QString &path, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QGitRepository)
    , m_path(path)
    , m_git(new QGit())
{
    ui->setupUi(this);

    connect(this, SIGNAL(repositoryBranches(QDir)), m_git, SLOT(repositoryBranches(QDir)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(repositoryBranchesReply(QList<QGitBranch>)), this, SLOT(repositoryBranchesReply(QList<QGitBranch>)), Qt::QueuedConnection);

    emit repositoryBranches(QDir(m_path));
}

QGitRepository::~QGitRepository()
{
    delete ui;
}

void QGitRepository::repositoryBranchesReply(QList<QGitBranch> branches)
{
    QList<QTreeWidgetItem *> items;
    QTreeWidgetItem *local = new QTreeWidgetItem();
    QTreeWidgetItem *remote = new QTreeWidgetItem();

    ui->branchesTreeView->clear();

    local->setText(0, tr("local"));
    remote->setText(0, tr("remote"));

    for(auto branch: branches)
    {
        QStringList items = branch.name.split('/');
        QTreeWidgetItem *branch_item = nullptr;

        switch(branch.type)
        {
        case GIT_BRANCH_LOCAL:
            branch_item = new QTreeWidgetItem(QStringList() << items.at(2));
            local->addChild(branch_item);
            break;
        case GIT_BRANCH_REMOTE:
            branch_item = new QTreeWidgetItem(QStringList() << items.at(3));
            remote->addChild(branch_item);
            break;
        case GIT_BRANCH_ALL:
            // TODO: Implement QGitRepository::repositoryBranchesReply - GIT_BRANCH_ALL
            qDebug() << "Unimplemented(GIT_BRANCH_ALL)";
            break;
        }
    }

    items.append(local);
    items.append(remote);

    ui->branchesTreeView->addTopLevelItems(items);

    ui->branchesTreeView->expandAll();
}
