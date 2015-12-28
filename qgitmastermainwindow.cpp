#include "qgitmastermainwindow.h"
#include "ui_qgitmastermainwindow.h"
#include "qgitrepotreeitemdelegate.h"
#include "qnewrepositorydialog.h"
#include "qgitrepository.h"

#include <QSettings>


QGitMasterMainWindow::QGitMasterMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QGitMasterMainWindow)
    , m_git(this)
{
    ui->setupUi(this);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->insertWidget(ui->actionSettings, spacer);

    ui->splitter->setSizes(QList<int>() << 1000 << 4000);

    QGitRepoTreeItemDelegate *treeDelegate = new QGitRepoTreeItemDelegate(this);

    treeDelegate->setImages(
                QImage(":/images/gray_branch"),
                QImage(":/images/file_ok"),
                QImage(":/images/file_new"),
                QImage(":/images/file_modified"),
                QImage(":/images/file_removed"),
                QImage(":/images/branch_icon"));

    ui->treeWidget->setItemDelegate(treeDelegate);

    readSettings();
}

QGitMasterMainWindow::~QGitMasterMainWindow()
{
    delete ui;
}

bool QGitMasterMainWindow::event(QEvent *event)
{
    if (event->type() == QEvent::WindowActivate)
    {
        ui->treeWidget->refreshItems();
    }

    return QMainWindow::event(event);
}

void QGitMasterMainWindow::readSettings()
{
    QSettings settings;

    //settings.setValue("0\\name", "{some name}");
    //settings.setValue("0\\path", "{some path}");
    //settings.setValue("1\\name", "{some name}");
    //settings.setValue("1\\path", "{some path}");
    //settings.setValue("2\\name", "{some name}");
    //settings.setValue("2\\path", "{some path}");

    for(int c = 0; ; c++)
    {
        const QString counterStr = QString::number(c);
        const QVariant name = settings.value(counterStr + "\\name");
        const QVariant path = settings.value(counterStr + "\\path");

        if ((name.isNull())||(path.isNull()))
        {
            break;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setData(0, Qt::DisplayRole, name);
        item->setData(0, QGitRepoTreeItemDelegate::QItemPath, path);

        ui->treeWidget->addTopLevelItem(item);
    }
}

void QGitMasterMainWindow::on_actionCloneNew_triggered()
{
    QNewRepositoryDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted)
    {
        QString bookmarkStr;

        switch(dlg.tabCurrentIndex())
        {
        case QNewRepositoryDialog::QCloneRepository:
            Q_UNIMPLEMENTED();
            break;
        case QNewRepositoryDialog::QAddWorkingCopy:
            Q_UNIMPLEMENTED();
            break;
        case QNewRepositoryDialog::QCreateNewRepository:
            QGit::createLocalRepository(QDir(dlg.createNewRepositoryPath()));

            bookmarkStr = dlg.createNewRepositoryBookmark();

            if(!bookmarkStr.isEmpty())
            {
                QTreeWidgetItem *item = new QTreeWidgetItem();

                item->setData(0, Qt::DisplayRole, bookmarkStr);
                item->setData(0, QGitRepoTreeItemDelegate::QItemPath, dlg.createNewRepositoryPath());

                ui->treeWidget->addTopLevelItem(item);
            }
            break;
        }
    }
}

void QGitMasterMainWindow::on_actionNewFolderRepository_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionDeleteRepository_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionSettingsRepository_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QString repositoryName = item->data(column, Qt::DisplayRole).toString();
    QString repositoryPath = item->data(column, QGitRepoTreeItemDelegate::QItemPath).toString();

    for(int index = 0; index < ui->tabWidget->count(); index++)
    {
        if (repositoryName == ui->tabWidget->tabText(index))
        {
            ui->tabWidget->setCurrentIndex(index);

            return;
        }
    }

    QGitRepository *widget = new QGitRepository(repositoryPath, this);

    ui->tabWidget->addTab(widget, repositoryName);

    ui->tabWidget->setCurrentWidget(widget);
}

void QGitMasterMainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->removeTab(index);
}
