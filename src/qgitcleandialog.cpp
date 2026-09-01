#include "qgitcleandialog.h"
#include "ui_qgitcleandialog.h"

QGitCleanDialog::QGitCleanDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitCleanDialog)
{
    ui->setupUi(this);
}

QGitCleanDialog::~QGitCleanDialog()
{
    delete ui;
}

bool QGitCleanDialog::removeDirectories() const
{
    return ui->removeDirectories_checkBox->isChecked();
}

bool QGitCleanDialog::removeIgnored() const
{
    return ui->removeIgnored_checkBox->isChecked();
}
