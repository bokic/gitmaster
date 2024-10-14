#include "qgitpulldialog.h"
#include "ui_qgitpulldialog.h"

QGitPullDialog::QGitPullDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitPullDialog)
{
    ui->setupUi(this);
}

QGitPullDialog::~QGitPullDialog()
{
    delete ui;
}
