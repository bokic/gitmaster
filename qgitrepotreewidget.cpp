#include "qgitrepotreewidget.h"

QGitRepoTreeWidget::QGitRepoTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
    , m_thread(this)
    , m_git(new QGit())
{
    m_git->moveToThread(&m_thread);

    connect(this, SIGNAL(repositoryStatus(QDir)), m_git, SLOT(repositoryStatus(QDir)), Qt::QueuedConnection);

    m_thread.start();
}

QGitRepoTreeWidget::~QGitRepoTreeWidget()
{
    m_thread.terminate();
    m_thread.wait();

    delete m_git; m_git = nullptr;
}

void QGitRepoTreeWidget::refreshItems()
{
    for(int index = 0; index < topLevelItemCount(); index++)
    {
        QTreeWidgetItem *item = topLevelItem(index);

        emit repositoryStatus(QDir(item->data(0, Qt::UserRole + 1).toString()));
    }
}
