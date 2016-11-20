#include "qnewrepositorydialog.h"
#include "qgitmastermainwindow.h"
#include "ui_qnewrepositorydialog.h"
#include "qgit.h"
#include <QFileDialog>
#include <QPushButton>

QNewRepositoryDialog::QNewRepositoryDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QNewRepositoryDialog)
{
    ui->setupUi(this);

    connect(ui->lineEditCloneRepositorySourceURL, SIGNAL(textChanged(QString)), SLOT(updateWidgets()));
    connect(ui->lineEditCloneRepositoryDestinationPath, SIGNAL(textChanged(QString)), SLOT(updateWidgets()));
    connect(ui->lineEditCloneRepositoryBookmark, SIGNAL(textChanged(QString)), SLOT(updateWidgets()));
    connect(ui->lineEditAddWorkingCopyPath, SIGNAL(textChanged(QString)), SLOT(updateWidgets()));
    connect(ui->lineEditAddWorkingCopyBookmark, SIGNAL(textChanged(QString)), SLOT(updateWidgets()));
    connect(ui->lineEditCreateNewRepositoryPath, SIGNAL(textChanged(QString)), SLOT(updateWidgets()));
    connect(ui->lineEditCreateNewRepositoryBookmark, SIGNAL(textChanged(QString)), SLOT(updateWidgets()));

    ui->lineEditCloneRepositoryBookmark->setEnabled(false);
    ui->lineEditAddWorkingCopyBookmark->setEnabled(false);
    ui->lineEditCreateNewRepositoryBookmark->setEnabled(false);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

QNewRepositoryDialog::~QNewRepositoryDialog()
{
    delete ui;
}

int QNewRepositoryDialog::tabCurrentIndex() const
{
    return ui->tabWidget->currentIndex();
}

QString QNewRepositoryDialog::cloneRepositorySourceURL() const
{
    return ui->lineEditCloneRepositorySourceURL->text();
}

QString QNewRepositoryDialog::cloneRepositoryDestinationPath() const
{
    return ui->lineEditCloneRepositoryDestinationPath->text();
}

QString QNewRepositoryDialog::cloneRepositoryBookmarkText() const
{
    if (ui->checkBoxCloneRepositoryBookmark->isChecked())
    {
        return ui->lineEditCloneRepositoryBookmark->text();
    }

    return QString();
}

QString QNewRepositoryDialog::addWorkingCopyPath() const
{
    return ui->lineEditAddWorkingCopyPath->text();
}

QString QNewRepositoryDialog::addWorkingCopyBookmarkText() const
{
    if (ui->checkBoxAddWorkingCopyBookmark->isChecked())
    {
        return ui->lineEditAddWorkingCopyBookmark->text();
    }

    return QString();
}

QString QNewRepositoryDialog::createNewRepositoryPath() const
{
    return ui->lineEditCreateNewRepositoryPath->text();
}

QString QNewRepositoryDialog::createNewRepositoryBookmarkText() const
{
    if (ui->checkBoxCreateNewRepositoryBookmark->isChecked())
    {
        return ui->lineEditCreateNewRepositoryBookmark->text();
    }

    return QString();
}

void QNewRepositoryDialog::updateWidgets()
{
    bool enable = false;
    QDir dir;

    switch(ui->tabWidget->currentIndex())
    {
    case QCloneRepository:
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

        dir = QDir(ui->lineEditCloneRepositoryDestinationPath->text());

        if ((dir.exists())&&(dir.entryInfoList().count() > 2))
        {
            ui->lineEditCloneRepositoryDestinationPath->setStyleSheet("background: red");
            ui->lineEditCloneRepositoryDestinationPath->setToolTip(tr("Path exist and not empty."));
            enable = false;
        }
        else
        {
            ui->lineEditCloneRepositoryDestinationPath->setStyleSheet("");
            ui->lineEditCloneRepositoryDestinationPath->setToolTip("");
        }

        if (ui->checkBoxCloneRepositoryBookmark->isChecked())
        {
            QString repositoryName = ui->lineEditCloneRepositoryBookmark->text();
            if (repositoryName.isEmpty())
            {
                ui->lineEditCloneRepositoryBookmark->setStyleSheet("background: red");
                ui->lineEditCloneRepositoryBookmark->setToolTip(tr("Bookmark name is empty."));
                enable = false;
            }
            else
            {
                QGitMasterMainWindow *mainWindow = static_cast<QGitMasterMainWindow *>(parent());

                if (mainWindow)
                {
                    if (mainWindow->hasRepositoryWithName(repositoryName))
                    {
                        ui->lineEditCloneRepositoryBookmark->setStyleSheet("background: red");
                        ui->lineEditCloneRepositoryBookmark->setToolTip(tr("Bookmark name already exist. Please choose another name."));
                        enable = false;
                    }
                    else
                    {
                        ui->lineEditCloneRepositoryBookmark->setStyleSheet("");
                        ui->lineEditCloneRepositoryBookmark->setToolTip("");
                    }
                }
            }
        }
        break;
    case QAddWorkingCopy:
        if (
                (!ui->lineEditAddWorkingCopyPath->text().isEmpty())
                &&
                (!((ui->checkBoxAddWorkingCopyBookmark->isChecked())&&(ui->lineEditAddWorkingCopyBookmark->text().isEmpty())))
            )
        {
            enable = true;
        }

        dir = QDir(ui->lineEditAddWorkingCopyPath->text());

        if (!dir.exists())
        {
            ui->lineEditAddWorkingCopyPath->setStyleSheet("background: red");
            ui->lineEditAddWorkingCopyPath->setToolTip(tr("Path doesn't exist."));
            enable = false;
        }
        else if (QGit::isGitRepository(dir) == false)
        {
            ui->lineEditAddWorkingCopyPath->setStyleSheet("background: red");
            ui->lineEditAddWorkingCopyPath->setToolTip(tr("Dir is not valid git repository."));
            enable = false;
        }
        else
        {
            ui->lineEditAddWorkingCopyPath->setStyleSheet("");
            ui->lineEditAddWorkingCopyPath->setToolTip("");
        }

        if (ui->checkBoxAddWorkingCopyBookmark->isChecked())
        {
            QString repositoryName = ui->lineEditAddWorkingCopyBookmark->text();
            if (repositoryName.isEmpty())
            {
                ui->lineEditAddWorkingCopyBookmark->setStyleSheet("background: red");
                ui->lineEditAddWorkingCopyBookmark->setToolTip(tr("Bookmark name is empty."));
                enable = false;
            }
            else
            {
                QGitMasterMainWindow *mainWindow = static_cast<QGitMasterMainWindow *>(parent());

                if (mainWindow)
                {
                    if (mainWindow->hasRepositoryWithName(repositoryName))
                    {
                        ui->lineEditAddWorkingCopyBookmark->setStyleSheet("background: red");
                        ui->lineEditAddWorkingCopyBookmark->setToolTip(tr("Bookmark name already exist. Please choose another name."));
                        enable = false;
                    }
                    else
                    {
                        ui->lineEditAddWorkingCopyBookmark->setStyleSheet("");
                        ui->lineEditAddWorkingCopyBookmark->setToolTip("");
                    }
                }
            }
        }
        break;
    case QCreateNewRepository:
        if (
                (!ui->lineEditCreateNewRepositoryPath->text().isEmpty())
                &&
                (!((ui->checkBoxCreateNewRepositoryBookmark->isChecked())&&(ui->lineEditCreateNewRepositoryBookmark->text().isEmpty())))
            )
        {
            enable = true;
        }

        dir = QDir(ui->lineEditCreateNewRepositoryPath->text());

        if (!dir.exists())
        {
            ui->lineEditCreateNewRepositoryPath->setStyleSheet("background: red");
            ui->lineEditCreateNewRepositoryPath->setToolTip(tr("Path doesn't exist."));
            enable = false;
        }
        else if (dir.entryInfoList().count() > 2)
        {
            ui->lineEditCreateNewRepositoryPath->setStyleSheet("background: red");
            ui->lineEditCreateNewRepositoryPath->setToolTip(tr("Dir is not empty."));
            enable = false;
        }
        else
        {
            ui->lineEditCreateNewRepositoryPath->setStyleSheet("");
            ui->lineEditCreateNewRepositoryPath->setToolTip("");
        }

        if (ui->checkBoxCreateNewRepositoryBookmark->isChecked())
        {
            QString repositoryName = ui->lineEditCreateNewRepositoryBookmark->text();
            if (repositoryName.isEmpty())
            {
                ui->lineEditCreateNewRepositoryBookmark->setStyleSheet("background: red");
                ui->lineEditCreateNewRepositoryBookmark->setToolTip(tr("Bookmark name is empty."));
                enable = false;
            }
            else
            {
                QGitMasterMainWindow *mainWindow = static_cast<QGitMasterMainWindow *>(parent());

                if (mainWindow)
                {
                    if (mainWindow->hasRepositoryWithName(repositoryName))
                    {
                        ui->lineEditCreateNewRepositoryBookmark->setStyleSheet("background: red");
                        ui->lineEditCreateNewRepositoryBookmark->setToolTip(tr("Bookmark name already exist. Please choose another name."));
                        enable = false;
                    }
                    else
                    {
                        ui->lineEditCreateNewRepositoryBookmark->setStyleSheet("");
                        ui->lineEditCreateNewRepositoryBookmark->setToolTip("");
                    }
                }
            }
        }

        break;
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enable);
}

void QNewRepositoryDialog::on_toolButtonCloneRepositoryBrowse_clicked()
{
    QString dirStr = QFileDialog::getExistingDirectory(this);

    updateWidgets();

    if (!dirStr.isEmpty())
    {
        QDir dir(dirStr);

        ui->lineEditCloneRepositoryDestinationPath->setText(QDir::toNativeSeparators(dirStr));

        ui->lineEditCloneRepositoryBookmark->setText(dir.dirName());
        ui->checkBoxCloneRepositoryBookmark->setChecked(true);
    }
}

void QNewRepositoryDialog::on_toolButtonAddWorkingCopyBrowse_clicked()
{
    QString dirStr = QFileDialog::getExistingDirectory(this);

    updateWidgets();

    if (!dirStr.isEmpty())
    {
        QDir dir(dirStr);

        ui->lineEditAddWorkingCopyPath->setText(QDir::toNativeSeparators(dirStr));

        if (dir.exists())
        {
            ui->lineEditAddWorkingCopyBookmark->setText(dir.dirName());
            ui->checkBoxAddWorkingCopyBookmark->setChecked(true);
        }
    }
}

void QNewRepositoryDialog::on_toolButtonCreateNewRepositoryBrowse_clicked()
{
    QString dirStr = QFileDialog::getExistingDirectory(this);

    updateWidgets();

    if (!dirStr.isEmpty())
    {
        QDir dir(dirStr);

        ui->lineEditCreateNewRepositoryPath->setText(QDir::toNativeSeparators(dirStr));

        if (dir.exists())
        {
            ui->lineEditCreateNewRepositoryBookmark->setText(dir.dirName());
            ui->checkBoxCreateNewRepositoryBookmark->setChecked(true);
        }
    }
}

void QNewRepositoryDialog::on_checkBoxCloneRepositoryBookmark_stateChanged(int state)
{
    bool checked = (state == Qt::Checked);

    ui->lineEditCloneRepositoryBookmark->setEnabled(checked);

    updateWidgets();

    if (checked)
    {
        ui->lineEditCloneRepositoryBookmark->setFocus();
    }
}

void QNewRepositoryDialog::on_checkBoxAddWorkingCopyBookmark_stateChanged(int state)
{
    bool checked = (state == Qt::Checked);

    ui->lineEditAddWorkingCopyBookmark->setEnabled(checked);

    updateWidgets();

    if (checked)
    {
        ui->lineEditAddWorkingCopyBookmark->setFocus();
    }
}

void QNewRepositoryDialog::on_checkBoxCreateNewRepositoryBookmark_stateChanged(int state)
{
    bool checked = (state == Qt::Checked);

    ui->lineEditCreateNewRepositoryBookmark->setEnabled(checked);

    updateWidgets();

    if (checked)
    {
        ui->lineEditCreateNewRepositoryBookmark->setFocus();
    }
}
