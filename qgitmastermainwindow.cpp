#include "qgitmastermainwindow.h"
#include "ui_qgitmastermainwindow.h"
#include "qgitrepotreeitemdelegate.h"
#include "qgitclonerepositorydialog.h"
#include "qnewrepositorydialog.h"
#include "qgitrepository.h"

#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QSettings>


QGitMasterMainWindow::QGitMasterMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QGitMasterMainWindow)
{
    ui->setupUi(this);

    QWidget *spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->mainToolBar->insertWidget(ui->actionSettings, spacer);

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

QGitMasterMainWindow *QGitMasterMainWindow::instance()
{
    foreach(const auto widget, QApplication::topLevelWidgets())
    {
        if (dynamic_cast<QGitMasterMainWindow *>(widget))
        {
            return dynamic_cast<QGitMasterMainWindow *>(widget);
        }
    }

    return nullptr;
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

    for(int c = 0; ; c++)
    {
        const QString counterStr = QString::number(c);
        const QVariant name = settings.value(counterStr + "\\name");
        const QVariant path = settings.value(counterStr + "\\path");

        if ((name.isNull())&&(path.isNull()))
        {
            break;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setData(0, Qt::DisplayRole, name);
        item->setData(0, QGitRepoTreeItemDelegate::QItemPath, path);

        ui->treeWidget->addTopLevelItem(item);
    }
}

void QGitMasterMainWindow::writeSettings()
{
    QSettings settings;

    // Delete out of range settings.
    for (int c = ui->treeWidget->topLevelItemCount(); ; c++)
    {
        const QString keyName = QString("%1\\name").arg(c);
        const QString keyPath = QString("%1\\path").arg(c);

        const QVariant name = settings.value(keyName);
        const QVariant path = settings.value(keyPath);

        if ((name.isNull())&&(path.isNull()))
        {
            break;
        }

        settings.remove(keyName);
        settings.remove(keyPath);
    }

    // Store settings
    for(int c = 0; c < ui->treeWidget->topLevelItemCount(); c++)
    {
        const QString keyName = QString("%1\\name").arg(c);
        const QString keyPath = QString("%1\\path").arg(c);

        QTreeWidgetItem *item = ui->treeWidget->topLevelItem(c);

        settings.setValue(keyName, item->data(0, Qt::DisplayRole));
        settings.setValue(keyPath, item->data(0, QGitRepoTreeItemDelegate::QItemPath));
    }
}

bool QGitMasterMainWindow::hasRepositoryWithName(const QString &name)
{
    for(int c = 0; c < ui->treeWidget->topLevelItemCount(); c++)
    {
        QString itemName = ui->treeWidget->topLevelItem(c)->data(0, Qt::DisplayRole).toString();

        if (itemName == name)
        {
            return true;
        }
    }

    return false;
}

void QGitMasterMainWindow::updateStatusBarText(const QString &text)
{
    if (!text.isEmpty())
        ui->statusBar->showMessage(text);
    else
        ui->statusBar->clearMessage();
}

void QGitMasterMainWindow::on_actionCloneNew_triggered()
{
    QNewRepositoryDialog dlg(this);
    QString bookmarkStr;

    if (dlg.exec() == QDialog::Accepted)
    {
        int currentTab = dlg.tabCurrentIndex();

        if ( currentTab == QNewRepositoryDialog::QCloneRepository )
        {
            QString path;
            QString url;
            int res = 0;

            bookmarkStr = dlg.cloneRepositoryBookmarkText();
            path = dlg.cloneRepositoryDestinationPath();
            url = dlg.cloneRepositorySourceURL();

            QGitCloneRepositoryDialog cloneDlg(url, path, this);

            res = cloneDlg.exec();

            if (res == QDialog::Accepted)
            {
                QTreeWidgetItem *item = new QTreeWidgetItem();

                item->setData(0, Qt::DisplayRole, bookmarkStr);
                item->setData(0, QGitRepoTreeItemDelegate::QItemPath, path);

                ui->treeWidget->addTopLevelItem(item);

                writeSettings();

                ui->treeWidget->refreshItems();
            }
        }
        else if ( currentTab == QNewRepositoryDialog::QAddWorkingCopy )
        {
            bookmarkStr = dlg.addWorkingCopyBookmarkText();

            if(!bookmarkStr.isEmpty())
            {
                QTreeWidgetItem *item = new QTreeWidgetItem();

                item->setData(0, Qt::DisplayRole, bookmarkStr);
                item->setData(0, QGitRepoTreeItemDelegate::QItemPath, dlg.addWorkingCopyPath());

                ui->treeWidget->addTopLevelItem(item);

                writeSettings();

                ui->treeWidget->refreshItems();
            }
        }
        else if ( currentTab == QNewRepositoryDialog::QCreateNewRepository )
        {
            QGit::createLocalRepository(QDir(dlg.createNewRepositoryPath()));

            bookmarkStr = dlg.createNewRepositoryBookmarkText();

            if(!bookmarkStr.isEmpty())
            {
                QTreeWidgetItem *item = new QTreeWidgetItem();

                item->setData(0, Qt::DisplayRole, bookmarkStr);
                item->setData(0, QGitRepoTreeItemDelegate::QItemPath, dlg.createNewRepositoryPath());

                ui->treeWidget->addTopLevelItem(item);

                writeSettings();

                ui->treeWidget->refreshItems();
            }
        }
    }
}

void QGitMasterMainWindow::on_actionNewFolderRepository_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionDeleteRepository_triggered()
{
    QTreeWidgetItem *item = ui->treeWidget->currentItem();

    if (item)
    {
        if (QMessageBox::question(this, tr("Remove git bookmark?"), tr("Remote git bookmark?\nIt will not remove files.")) == QMessageBox::Yes)
        {
            ui->treeWidget->takeTopLevelItem(ui->treeWidget->indexOfTopLevelItem(item));

            writeSettings();

            delete item;
        }
    }
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
