// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitbranchdialog.h"
#include "ui_qgitbranchdialog.h"
#include "qgitcommitpickerdialog.h"
#include "qgitrepository.h"
#include "qgit.h"

#include <QSignalBlocker>
#include <git2.h>

namespace {

QString shortBranchName(const QString &name)
{
    if (name.startsWith(QStringLiteral("refs/heads/")))
        return name.mid(11);
    return name;
}

const QString &errorInputStyle()
{
    static const QString style = QStringLiteral("background-color: rgba(220, 53, 69, 0.35);");
    return style;
}

} // namespace

QGitBranchDialog::QGitBranchDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitBranchDialog)
{
    ui->setupUi(this);

    if (parent && parent->git())
        m_git.setPath(parent->git()->path());

    ui->deleteBranches_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->currentBranch_label->setText(m_git.currentBranch());
    m_currentBranches = m_git.branches(GIT_BRANCH_LOCAL);

    m_commitValidationTimer = new QTimer(this);
    m_commitValidationTimer->setSingleShot(true);
    m_commitValidationTimer->setInterval(250);
    connect(m_commitValidationTimer, &QTimer::timeout, this, &QGitBranchDialog::validateCommitId);

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
    return shortBranchName(ui->newBranch_lineEdit->text().trimmed());
}

QString QGitBranchDialog::newBranchCommitId() const
{
    if (ui->workingCopy_radioButton->isChecked())
        return QString();
    return ui->newBranchCommitId_lineEdit->text().trimmed();
}

bool QGitBranchDialog::newBranchCheckout() const
{
    return ui->newBranchCheckout_checkBox->isChecked();
}

void QGitBranchDialog::accept()
{
    if (m_commitValidationTimer && m_commitValidationTimer->isActive())
    {
        m_commitValidationTimer->stop();
        validateCommitId();
    }

    if (operation() == NewBranchOperation && !ui->createBranch_pushButton->isEnabled())
    {
        return;
    }

    if (operation() == DeleteBranchesOperation && !ui->deleteBranches_pushButton->isEnabled())
    {
        return;
    }

    QDialog::accept();
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
    ui->deleteBranches_pushButton->setEnabled(false);

    const QSignalBlocker blocker(ui->deleteBranches_tableWidget);
    auto branches = m_git.branches(GIT_BRANCH_ALL);

    ui->deleteBranches_tableWidget->setRowCount(static_cast<int>(branches.size()));

    const QString currentBranch = shortBranchName(m_git.currentBranch());

    for(int c = 0; c < branches.size(); c++)
    {
        const auto &branch = branches.at(c);
        const QString shortName = shortBranchName(branch.name());

        auto nameItem = new QTableWidgetItem(branch.name());
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
        nameItem->setCheckState(Qt::Unchecked);

        if (shortName == currentBranch)
        {
            auto font = nameItem->font();
            font.setBold(true);
            nameItem->setFont(font);
            nameItem->setData(Qt::ToolTipRole, tr("Can't delete current branch."));
        }

        if ((shortName == currentBranch) || (shortName == QStringLiteral("master")) || (shortName == QStringLiteral("main")))
        {
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEnabled);
        }

        nameItem->setData(Qt::UserRole, branch.type());
        nameItem->setData(Qt::UserRole + 1, static_cast<qlonglong>(branch.time()));

        ui->deleteBranches_tableWidget->setItem(c, 0, nameItem);

        auto typeItem = new QTableWidgetItem();
        typeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

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

void QGitBranchDialog::on_pushButton_clicked()
{
    QGitCommitPickerDialog dlg(m_git.path(), this);
    if (dlg.exec() == QDialog::Accepted) {
        const QString commitId = dlg.selectedCommitId();
        if (!commitId.isEmpty()) {
            ui->specifiedCommit_radioButton->setChecked(true);
            ui->newBranchCommitId_lineEdit->setText(commitId);
            validateCommitId();
        }
    }
}

void QGitBranchDialog::on_workingCopy_radioButton_toggled(bool checked)
{
    Q_UNUSED(checked)
    checkForNewBranch();
}

void QGitBranchDialog::on_specifiedCommit_radioButton_toggled(bool checked)
{
    Q_UNUSED(checked)
    checkForNewBranch();
}

void QGitBranchDialog::on_newBranch_lineEdit_textChanged(const QString &text)
{
    Q_UNUSED(text);

    checkForNewBranch();
}

void QGitBranchDialog::on_newBranchCommitId_lineEdit_textChanged(const QString &text)
{
    QString commitId = text.trimmed();
    if (commitId.isEmpty())
    {
        m_commitValidationTimer->stop();
        m_commitIdValid = true;
        ui->newBranchCommitId_lineEdit->setStyleSheet(QString());
        checkForNewBranch();
    }
    else
    {
        ui->specifiedCommit_radioButton->setChecked(true);
        m_commitValidationTimer->start(250);
        checkForNewBranch();
    }
}

void QGitBranchDialog::validateCommitId()
{
    QString commitId = ui->newBranchCommitId_lineEdit->text().trimmed();
    if (commitId.isEmpty())
    {
        m_commitIdValid = true;
        ui->newBranchCommitId_lineEdit->setStyleSheet(QString());
    }
    else
    {
        m_commitIdValid = m_git.hasCommitId(commitId);
        if (m_commitIdValid)
        {
            ui->newBranchCommitId_lineEdit->setStyleSheet(QString());
        }
        else
        {
            ui->newBranchCommitId_lineEdit->setStyleSheet(errorInputStyle());
        }
    }
    checkForNewBranch();
}

void QGitBranchDialog::checkForNewBranch()
{
    bool enable = true;

    const QString newBranch = shortBranchName(ui->newBranch_lineEdit->text().trimmed());

    if (!newBranch.isEmpty())
    {
        bool alreadyExists = false;
        for (const auto &branch : std::as_const(m_currentBranches))
        {
            if (shortBranchName(branch.name()) == newBranch)
            {
                alreadyExists = true;
                break;
            }
        }

        int refValid = 0;
        const QByteArray fullRef = (QStringLiteral("refs/heads/") + newBranch).toUtf8();
        const bool validRefName = (git_reference_name_is_valid(&refValid, fullRef.constData()) == 0) && (refValid != 0);

        if (alreadyExists)
        {
            ui->newBranch_lineEdit->setStyleSheet(errorInputStyle());
            ui->newBranch_lineEdit->setToolTip(tr("Local branch with name [%1] already exists.").arg(newBranch));
            enable = false;
        }
        else if (!validRefName)
        {
            ui->newBranch_lineEdit->setStyleSheet(errorInputStyle());
            ui->newBranch_lineEdit->setToolTip(tr("Invalid git branch name."));
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
        ui->newBranch_lineEdit->setStyleSheet(QString());
        ui->newBranch_lineEdit->setToolTip(QString());
        enable = false;
    }

    if (ui->specifiedCommit_radioButton->isChecked())
    {
        const QString commitId = ui->newBranchCommitId_lineEdit->text().trimmed();
        if (commitId.isEmpty())
        {
            enable = false;
        }
        else if (m_commitValidationTimer && m_commitValidationTimer->isActive())
        {
            enable = false;
        }
        else if (!m_commitIdValid)
        {
            enable = false;
        }
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
        const auto *rowItem = ui->deleteBranches_tableWidget->item(row, 0);
        if ((rowItem) && (rowItem->checkState() == Qt::Checked))
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
        const QTableWidgetItem *nameItem = ui->deleteBranches_tableWidget->item(c, 0);
        if (nameItem && nameItem->checkState() == Qt::Checked)
        {
            auto name = nameItem->text();
            auto time = nameItem->data(Qt::UserRole + 1).toLongLong();
            auto type = nameItem->data(Qt::UserRole).toInt();
            QGitBranch item(name, "", time, static_cast<git_branch_t>(type));

            ret.append(item);
        }
    }

    return ret;
}

bool QGitBranchDialog::forceDelete() const
{
    return ui->forcDelete_checkBox->isChecked();
}
