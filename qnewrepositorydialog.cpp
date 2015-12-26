#include "qnewrepositorydialog.h"
#include "ui_qnewrepositorydialog.h"
#include <QFileDialog>
#include <QPushButton>

QNewRepositoryDialog::QNewRepositoryDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QNewRepositoryDialog)
{
    ui->setupUi(this);

    connect(ui->lineEditCloneRepositorySourceURL, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));
    connect(ui->lineEditCloneRepositoryDestinationPath, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));
    connect(ui->lineEditCloneRepositoryBookmark, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));
    connect(ui->lineEditAddWorkingCopyPath, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));
    connect(ui->lineEditAddWorkingCopyBookmark, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));
    connect(ui->lineEditCreateNewRepositoryPath, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));
    connect(ui->lineEditCreateNewRepositoryBookmark, SIGNAL(textChanged(QString)), SLOT(updateOkButton()));

    ui->lineEditCloneRepositoryBookmark->setEnabled(false);
    ui->lineEditAddWorkingCopyBookmark->setEnabled(false);
    ui->lineEditCreateNewRepositoryBookmark->setEnabled(false);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

QNewRepositoryDialog::~QNewRepositoryDialog()
{
    delete ui;
}

void QNewRepositoryDialog::updateOkButton()
{
    bool enable = false;

    switch(ui->tabWidget->currentIndex())
    {
    case 0:
        if (
                (!ui->lineEditCloneRepositorySourceURL->text().isEmpty())
                &&
                (!ui->lineEditCloneRepositoryDestinationPath->text().isEmpty())
                &&
                (!((ui->checkBoxCloneRepositoryBookmark->isChecked())&&(ui->lineEditCloneRepositoryBookmark->text().isEmpty())))
            )
        {
            enable = true;
        }
        break;
    case 1:
        if (
                (!ui->lineEditAddWorkingCopyPath->text().isEmpty())
                &&
                (!((ui->checkBoxAddWorkingCopyBookmark->isChecked())&&(ui->lineEditAddWorkingCopyBookmark->text().isEmpty())))
            )
        {
            enable = true;
        }
        break;
    case 2:
        if (
                (!ui->lineEditCreateNewRepositoryPath->text().isEmpty())
                &&
                (!((ui->checkBoxCreateNewRepositoryBookmark->isChecked())&&(ui->lineEditCreateNewRepositoryBookmark->text().isEmpty())))
            )
        {
            enable = true;
        }
        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void QNewRepositoryDialog::on_toolButtonCloneRepositoryBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this);

    updateOkButton();

    if (!dir.isEmpty())
    {
        ui->lineEditCloneRepositoryDestinationPath->setText(QDir::toNativeSeparators(dir));
    }
}

void QNewRepositoryDialog::on_toolButtonAddWorkingCopyBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this);

    updateOkButton();

    if (!dir.isEmpty())
    {
        ui->lineEditAddWorkingCopyPath->setText(QDir::toNativeSeparators(dir));
    }
}

void QNewRepositoryDialog::on_toolButtonCreateNewRepositoryBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this);

    updateOkButton();

    if (!dir.isEmpty())
    {
        ui->lineEditCreateNewRepositoryPath->setText(QDir::toNativeSeparators(dir));
    }
}

void QNewRepositoryDialog::on_checkBoxCloneRepositoryBookmark_clicked(bool checked)
{
    ui->lineEditCloneRepositoryBookmark->setEnabled(checked);

    updateOkButton();

    if (checked)
    {
        ui->lineEditCloneRepositoryBookmark->setFocus();
    }
}

void QNewRepositoryDialog::on_checkBoxAddWorkingCopyBookmark_clicked(bool checked)
{
    ui->lineEditAddWorkingCopyBookmark->setEnabled(checked);

    updateOkButton();

    if (checked)
    {
        ui->lineEditAddWorkingCopyBookmark->setFocus();
    }
}

void QNewRepositoryDialog::on_checkBoxCreateNewRepositoryBookmark_clicked(bool checked)
{
    ui->lineEditCreateNewRepositoryBookmark->setEnabled(checked);

    updateOkButton();

    if (checked)
    {
        ui->lineEditCreateNewRepositoryBookmark->setFocus();
    }
}
