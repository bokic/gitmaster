#include "qgitstashdialog.h"
#include "ui_qgitstashdialog.h"

QGitStashDialog::QGitStashDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitStashDialog)
{
    ui->setupUi(this);
}

QGitStashDialog::~QGitStashDialog()
{
    delete ui;
}

QString QGitStashDialog::message() const
{
    return ui->lineEdit_message->text();
}

bool QGitStashDialog::keepIndex() const
{
    return ui->checkBox_keepIndex->isChecked();
}

bool QGitStashDialog::includeUntracked() const
{
    return ui->checkBox_includeUntracked->isChecked();
}

bool QGitStashDialog::includeIgnored() const
{
    return ui->checkBox_includeIgnored->isChecked();
}
