#include "qgitbranchdialog.h"
#include "ui_qgitbranchdialog.h"

QGitBranchDialog::QGitBranchDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitBranchDialog)
{
    ui->setupUi(this);
}

QGitBranchDialog::~QGitBranchDialog()
{
    delete ui;
}
