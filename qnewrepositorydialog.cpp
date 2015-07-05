#include "qnewrepositorydialog.h"
#include "ui_qnewrepositorydialog.h"
#include <QFileDialog>

QNewRepositoryDialog::QNewRepositoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QNewRepositoryDialog)
{
    ui->setupUi(this);
}

QNewRepositoryDialog::~QNewRepositoryDialog()
{
    delete ui;
}

void QNewRepositoryDialog::on_toolButtonCloneRepositoryBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this);

    if (!dir.isEmpty())
    {
        ui->lineEditCloneRepositoryDestinationPath->setText(QDir::toNativeSeparators(dir));
    }
}

void QNewRepositoryDialog::on_toolButtonAddWorkingCopyBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this);

    if (!dir.isEmpty())
    {
        ui->lineEditAddWorkingCopyPath->setText(QDir::toNativeSeparators(dir));
    }
}

void QNewRepositoryDialog::on_toolButtonCreateNewRepositoryBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this);

    if (!dir.isEmpty())
    {
        ui->lineEditCreateNewRepositoryPath->setText(QDir::toNativeSeparators(dir));
    }
}
