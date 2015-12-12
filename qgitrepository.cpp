#include "qgitrepository.h"
#include "ui_qgitrepository.h"

QGitRepository::QGitRepository(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::QGitRepository)
    , m_git(new QGit())
{
    ui->setupUi(this);

    connect(this, SIGNAL(repositoryBranches(QDir)), m_git, SLOT(repositoryBranches(QDir)), Qt::QueuedConnection);
    connect(m_git, SIGNAL(repositoryBranchesReply(QList<QGitBranch>)), this, SLOT(repositoryBranchesReply(QList<QGitBranch>)), Qt::QueuedConnection);
}

QGitRepository::~QGitRepository()
{
    delete ui;
}

void QGitRepository::repositoryBranchesReply(QList<QGitBranch> branches)
{
    return;
}
