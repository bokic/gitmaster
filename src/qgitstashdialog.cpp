// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitstashdialog.h"
#include "ui_qgitstashdialog.h"

QGitStashDialog::QGitStashDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitStashDialog)
{
    ui->setupUi(this);
    ui->okButton->setDefault(true);
    ui->lineEdit_message->setFocus();
}

QGitStashDialog::~QGitStashDialog()
{
    delete ui;
}

QString QGitStashDialog::message() const
{
    return ui->lineEdit_message->text().trimmed();
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
