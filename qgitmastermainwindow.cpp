#include "qgitmastermainwindow.h"
#include "ui_qgitmastermainwindow.h"
#include "qgitrepotreeitemdelegate.h"

#include <QSettings>


QGitMasterMainWindow::QGitMasterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QGitMasterMainWindow),
    m_git(this)
{
    ui->setupUi(this);

    ui->splitter->setSizes(QList<int>() << 1000 << 4000);

    QGitRepoTreeItemDelegate *treeDelegate = new QGitRepoTreeItemDelegate(this);

    treeDelegate->setImages(
                QImage(":/images/gray_branch"),
                QImage(":/images/file_add"),
                QImage(":/images/file_modified"),
                QImage(":/images/file_remove"),
                QImage(":/images/file_unknown"),
                QImage(":/images/branch_icon"));

    ui->treeWidget->setItemDelegate(treeDelegate);

    readSettings();
}

QGitMasterMainWindow::~QGitMasterMainWindow()
{
    delete ui;
}


void QGitMasterMainWindow::readSettings()
{
    QSettings settings;

    for(int c = 0; ; c++)
    {
        const QString &counterStr = QString::number(c);
        const QVariant &name = settings.value(counterStr + "\\name");
        const QVariant &path = settings.value(counterStr + "\\path");

        if ((name.isNull())||(path.isNull()))
        {
            break;
        }


        QTreeWidgetItem *item = new QTreeWidgetItem();

        item->setData(0, Qt::DisplayRole, name);
        item->setData(0, Qt::UserRole + 1, path);

        ui->treeWidget->addTopLevelItem(item);
    }
}
