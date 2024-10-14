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
