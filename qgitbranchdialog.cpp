#include "qgitbranchdialog.h"
#include "ui_qgitbranchdialog.h"
#include "qgitrepository.h"
#include "qgit.h"


QGitBranchDialog::QGitBranchDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitBranchDialog)
{
    ui->setupUi(this);

    m_git.setPath(parent->git()->path());

    ui->deleteBranches_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->currentBranch_label->setText(m_git.currentBranch());
    m_currentBranches = m_git.branches(GIT_BRANCH_LOCAL);

    m_git.branches(GIT_BRANCH_LOCAL);

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


    auto branches = m_git.branches(GIT_BRANCH_ALL);

    ui->deleteBranches_tableWidget->setRowCount(branches.count());

    auto currentBranch = m_git.currentBranch();


    for(int c = 0; c < branches.count(); c++)
    {
        const auto &branch = branches.at(c);

        auto nameItem = new QTableWidgetItem(branch.name());
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        nameItem->setCheckState(Qt::Unchecked);

        if (branch.name() == currentBranch)
        {
            auto font = nameItem->font();
            font.setBold(true);
            nameItem->setFont(font);
            nameItem->setData(Qt::ToolTipRole, tr("Can't delete current branch."));
        }

        if ((branch.name() == currentBranch)||(branch.name() == "master")||(branch.name() == "main"))
        {
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEnabled);
        }

        nameItem->setData(Qt::UserRole, branch.type());
        nameItem->setData(Qt::UserRole + 1, static_cast<qlonglong>(branch.time()));

        ui->deleteBranches_tableWidget->setItem(c, 0, nameItem);

        auto typeItem = new QTableWidgetItem();
        typeItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);

        switch (branch.type())
        {
        case GIT_BRANCH_LOCAL:
            typeItem->setText(tr("Local"));
            break;
        case GIT_BRANCH_REMOTE:
            typeItem->setText(tr("Remote"));
            break;
        default:
            typeItem->setText(tr("Unknown"));
        }

        ui->deleteBranches_tableWidget->setItem(c, 1, typeItem);
    }
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
        bool alreadyExists = false;

        for(const auto &branch: std::as_const(m_currentBranches))
        {
            if (branch.name() == newBranch)
            {
                alreadyExists = true;
                break;
            }
        }

        if (alreadyExists)
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

        if (m_git.hasCommitId(commitId))
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

void QGitBranchDialog::on_deleteBranches_tableWidget_itemChanged(QTableWidgetItem *item)
{
    bool enable = false;

    Q_UNUSED(item);

    int rows = ui->deleteBranches_tableWidget->rowCount();
    for(int row = 0; row < rows; row++)
    {
        const auto *item = ui->deleteBranches_tableWidget->item(row, 0);
        if ((item)&&(item->checkState() == Qt::Checked))
        {
            enable = true;
            break;
        }
    }

    ui->deleteBranches_pushButton->setEnabled(enable);
}

QList<QGitBranch> QGitBranchDialog::deleteBranches() const
{
    QList<QGitBranch> ret;

    for(int c = 0; c < ui->deleteBranches_tableWidget->rowCount(); c++)
    {
        if (ui->deleteBranches_tableWidget->item(c, 0)->checkState() == Qt::Checked)
        {
            auto name = ui->deleteBranches_tableWidget->item(c, 0)->text();
            auto time = ui->deleteBranches_tableWidget->item(c, 0)->data(Qt::UserRole + 1).toLongLong();
            auto type = ui->deleteBranches_tableWidget->item(c, 0)->data(Qt::UserRole).toInt();
            QGitBranch item(name, time, static_cast<git_branch_t>(type));

            ret.append(item);
        }
    }

    return ret;
}

bool QGitBranchDialog::forceDelete() const
{
    return ui->forcDelete_checkBox->isChecked();
}
