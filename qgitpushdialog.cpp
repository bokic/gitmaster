#include "qgitpushdialog.h"
#include "ui_qgitpushdialog.h"

QGitPushDialog::QGitPushDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitPushDialog)
{
    ui->setupUi(this);
}

QGitPushDialog::~QGitPushDialog()
{
    delete ui;
}
