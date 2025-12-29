#include "qgitbranchdialog.h"
#include "ui_qgitbranchdialog.h"
#include "qgitrepository.h"
#include "qgit.h"


QGitBranchDialog::QGitBranchDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitBranchDialog)
{
    ui->setupUi(this);

    QGit *git = static_cast<QGitRepository *>(parent)->git();

    ui->currentBranch_label->setText(git->localBranch());
    m_currentBranches = git->localBranches();
    ui->newBranch_lineEdit->setFocus();
}

QGitBranchDialog::~QGitBranchDialog()
{
    delete ui;
}

QGitBranchDialog::Operation QGitBranchDialog::operation() const
{
    switch (ui->stackedWidget->currentIndex())
    {
    case 0:
        return NewBranchOperation;
    case 1:
        return DeleteBranchesOperation;
    default:;
    }

    return UnknownOperation;
}

QString QGitBranchDialog::newBranchName() const
{
    return ui->newBranch_lineEdit->text();
}

QString QGitBranchDialog::newBranchCommitId() const
{
    return ui->newBranchCommitId_lineEdit->text();
}

bool QGitBranchDialog::newBranchCheckout() const
{
    return ui->newBranchCheckout_checkBox->isChecked();
}

void QGitBranchDialog::on_newBranch_pushButton_clicked()
{
    ui->newBranch_pushButton->setChecked(true);
    ui->deleteBranch_pushButton->setChecked(false);
    ui->stackedWidget->setCurrentIndex(0);
    ui->operation_label->setText(tr("New Branch"));

    checkForNewBranch();
}

void QGitBranchDialog::on_deleteBranch_pushButton_clicked()
{
    ui->deleteBranch_pushButton->setChecked(true);
    ui->newBranch_pushButton->setChecked(false);
    ui->stackedWidget->setCurrentIndex(1);
    ui->operation_label->setText(tr("Delete Branches"));
}

void QGitBranchDialog::on_newBranch_lineEdit_textChanged(const QString &text)
{
    Q_UNUSED(text);

    checkForNewBranch();
}

void QGitBranchDialog::on_newBranchCommitId_lineEdit_textChanged(const QString &text)
{
    Q_UNUSED(text);

    checkForNewBranch();
}

void QGitBranchDialog::checkForNewBranch()
{
    bool enable = true;

    QString newBranch = ui->newBranch_lineEdit->text();
    if (!newBranch.isEmpty())
    {
        if (m_currentBranches.contains(newBranch))
        {
            ui->newBranch_lineEdit->setStyleSheet("background: red");
            ui->newBranch_lineEdit->setToolTip(tr("Local branch with name [%1] already exists.").arg(newBranch));
            enable = false;
        }
        else
        {
            ui->newBranch_lineEdit->setStyleSheet(QString());
            ui->newBranch_lineEdit->setToolTip(QString());
        }
    }
    else
    {
        enable = false;
    }

    QString commitId = ui->newBranchCommitId_lineEdit->text();
    if (!commitId.isEmpty())
    {
        ui->specifiedCommit_radioButton->setChecked(true);

        QGit *git = static_cast<QGitRepository *>(parent())->git();

        if (git->hasCommitId(commitId))
        {
            ui->newBranchCommitId_lineEdit->setStyleSheet("");
        }
        else
        {
            ui->newBranchCommitId_lineEdit->setStyleSheet("background: red");
            enable = false;
        }
    }
    else
    {
        ui->workingCopy_radioButton->setChecked(true);
    }

    ui->createBranch_pushButton->setEnabled(enable);
}
