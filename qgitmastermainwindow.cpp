#include "qgitmastermainwindow.h"
#include "ui_qgitmastermainwindow.h"
#include "qgitrepotreeitemdelegate.h"
#include "qgitclonerepositorydialog.h"
#include "qnewrepositorydialog.h"
#include "qgitrepository.h"

#include <QTreeWidgetItem>
#include <QStyleFactory>
#include <QInputDialog>
#include <QActionGroup>
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

    auto treeDelegate = new QGitRepoTreeItemDelegate(this);

    treeDelegate->setImages(
                QImage(":/small/git"),
                QImage(":/small/clean"),
                QImage(":/small/unknown"),
                QImage(":/small/modified"),
                QImage(":/small/deleted"),
                QImage(":/images/branch_icon"));

    ui->treeWidget->setItemDelegate(treeDelegate);

    readSettings();

    auto themes = QStyleFactory::keys();

    if ((m_appTheme.isEmpty())||(!themes.contains(m_appTheme)))
        m_appTheme = QApplication::style()->name();
    else
        QApplication::setStyle(QStyleFactory::create(m_appTheme));

    auto themeGroup = new QActionGroup(this);

    for(const auto &theme: themes)
    {
        /*QAction *newAction = new QAction(ui->menu_Themes);

        newAction->setText(theme);
        newAction->setCheckable(true);
        themeGroup->addAction(newAction);

        if (m_appTheme.compare(theme, Qt::CaseInsensitive) == 0)
        {
            newAction->setChecked(true);
        }

        ui->menu_Themes->addAction(newAction);

        connect(newAction, &QAction::triggered, this, &QGitMasterMainWindow::change_theme_triggered);*/
    }
}

QGitMasterMainWindow::~QGitMasterMainWindow()
{
    delete ui;
}

QGitMasterMainWindow *QGitMasterMainWindow::instance()
{
    auto topLevelWidgets = QApplication::topLevelWidgets();
    for(const auto widget: topLevelWidgets)
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
        on_tabWidget_currentChanged(-1);
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

        auto item = new QTreeWidgetItem();

        item->setData(0, Qt::DisplayRole, name);
        item->setData(0, QGitRepoTreeItemDelegate::QItemPath, path);

        ui->treeWidget->addTopLevelItem(item);
    }

    m_appTheme = settings.value("theme").toString();
}

void QGitMasterMainWindow::writeSettings()
{
    QSettings settings;

    // Delete out of range settings.
    for(int c = ui->treeWidget->topLevelItemCount(); ; c++)
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
                auto item = new QTreeWidgetItem();

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
                auto item = new QTreeWidgetItem();

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
                auto item = new QTreeWidgetItem();

                item->setData(0, Qt::DisplayRole, bookmarkStr);
                item->setData(0, QGitRepoTreeItemDelegate::QItemPath, dlg.createNewRepositoryPath());

                ui->treeWidget->addTopLevelItem(item);

                writeSettings();

                ui->treeWidget->refreshItems();
            }
        }
    }
}


void QGitMasterMainWindow::on_actionOpen_triggered()
{
    Q_UNIMPLEMENTED();
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

void QGitMasterMainWindow::on_actionOpen_Bookmarked_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionUndo_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionRedo_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionCut_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionCopy_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionPaste_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionSelect_All_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionShow_Hide_Bookmarks_triggered()
{
    Q_UNIMPLEMENTED();
}


void QGitMasterMainWindow::on_actionShow_Hosted_Repositories_triggered()
{
    Q_UNIMPLEMENTED();
}


void QGitMasterMainWindow::on_actionRefresh_triggered()
{
    Q_UNIMPLEMENTED();
}


void QGitMasterMainWindow::on_actionClose_Tab_triggered()
{
    Q_UNIMPLEMENTED();
}


void QGitMasterMainWindow::on_actionNext_Tab_triggered()
{
    Q_UNIMPLEMENTED();
}


void QGitMasterMainWindow::on_actionPrevious_Tab_triggered()
{
    Q_UNIMPLEMENTED();
}

void QGitMasterMainWindow::on_actionFull_Screen_triggered()
{
    if (ui->actionFull_Screen->isChecked())
    {
        m_prevWindowState = windowState();
        setWindowState(Qt::WindowFullScreen);
    }
    else
    {
        setWindowState(m_prevWindowState);
    }
}


void QGitMasterMainWindow::on_actionFetch_triggered()
{
    auto panel = dynamic_cast<QGitRepository *>(ui->tabWidget->currentWidget());
    if (panel)
    {
        panel->fetch();
        panel->refreshData();
    }
}

void QGitMasterMainWindow::on_actionPull_triggered()
{
    auto panel = dynamic_cast<QGitRepository *>(ui->tabWidget->currentWidget());
    if (panel)
    {
        panel->pull();
        panel->refreshData();
    }
}

void QGitMasterMainWindow::on_actionPush_triggered()
{
    auto panel = dynamic_cast<QGitRepository *>(ui->tabWidget->currentWidget());
    if (panel)
    {
        panel->push();
        panel->refreshData();
    }
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

    auto widget = new QGitRepository(repositoryPath, this);

    ui->tabWidget->addTab(widget, repositoryName);

    ui->tabWidget->setCurrentWidget(widget);
}

void QGitMasterMainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->removeTab(index);
}

void QGitMasterMainWindow::on_tabWidget_currentChanged(int index)
{
    Q_UNUSED(index)

    auto panel = dynamic_cast<QGitRepository *>(ui->tabWidget->currentWidget());
    if (panel)
    {
        panel->refreshData();
    }
}

void QGitMasterMainWindow::on_actionStash_triggered()
{
    auto widget = dynamic_cast<QGitRepository *>(ui->tabWidget->currentWidget());
    if (widget) {
        QString stashName = QInputDialog::getText(this, tr("Question?"), tr("Stash name:"));

        if(!stashName.isEmpty()) {
            widget->stash(stashName);
        }
    }
}

void QGitMasterMainWindow::change_theme_triggered(bool checked)
{
    QAction *menu = nullptr;
    QSettings settings;

    menu = dynamic_cast<QAction *>(sender());
    if (menu)
    {
        auto name = menu->text();

        QApplication::setStyle(QStyleFactory::create(name));
        menu->setChecked(true);

        settings.setValue("theme", name);
    }
}
