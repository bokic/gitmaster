#include "qgitfetchdialog.h"
#include "ui_qgitfetchdialog.h"


QGitFetchDialog::QGitFetchDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitFetchDialog)
{
    ui->setupUi(this);
}

QGitFetchDialog::~QGitFetchDialog()
{
    delete ui;
}

bool QGitFetchDialog::fetchFromAllRemotes() const
{
    return ui->checkBox_fetchFromAllRemotes->isChecked();
}

bool QGitFetchDialog::purgeDeletedBranches() const
{
    return ui->checkBox_PurgeDeletedBranches->isChecked();
}

bool QGitFetchDialog::fetchAllTags() const
{
    return ui->checkBox_fetchAllTags->isChecked();
}
