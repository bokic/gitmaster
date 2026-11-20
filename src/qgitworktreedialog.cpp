// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qgitworktreedialog.h"
#include "ui_qgitworktreedialog.h"
#include "qgitrepository.h"
#include "qgit.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QPushButton>
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

QGitWorktreeDialog::QGitWorktreeDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitWorktreeDialog)
{
    ui->setupUi(this);

    if (parent && parent->git())
        m_git.setPath(parent->git()->path());

    QSet<QString> checkedOutBranches;
    const QString currentBranch = shortBranchName(m_git.currentBranch());
    if (!currentBranch.isEmpty())
        checkedOutBranches.insert(currentBranch);

    const auto worktrees = m_git.worktrees();
    for (const auto &wt : worktrees) {
        if (!wt.name().isEmpty())
            m_existingWorktreeNames.insert(wt.name());
        const QString wtBranch = shortBranchName(wt.branch());
        if (!wtBranch.isEmpty())
            checkedOutBranches.insert(wtBranch);
    }

    // Populate branch combobox with local branches not already checked out
    const auto branches = m_git.branches(GIT_BRANCH_LOCAL);
    for (const auto &branch : branches) {
        const QString shortName = shortBranchName(branch.name());
        m_existingBranchNames.insert(shortName);
        if (!checkedOutBranches.contains(shortName)) {
            ui->comboBox_branch->addItem(shortName);
        }
    }

    if (ui->comboBox_branch->count() == 0) {
        ui->checkBox_newBranch->setChecked(true);
        ui->comboBox_branch->setEnabled(false);
    }

    ui->lineEdit_name->setFocus();
    validate();
}

QGitWorktreeDialog::~QGitWorktreeDialog()
{
    delete ui;
}

QString QGitWorktreeDialog::worktreeName() const
{
    return ui->lineEdit_name->text().trimmed();
}

QString QGitWorktreeDialog::worktreePath() const
{
    return ui->lineEdit_path->text().trimmed();
}

QString QGitWorktreeDialog::worktreeBranch() const
{
    if (ui->checkBox_newBranch->isChecked())
        return ui->lineEdit_name->text().trimmed(); // new branch = same name as worktree
    return ui->comboBox_branch->currentText();
}

bool QGitWorktreeDialog::createNewBranch() const
{
    return ui->checkBox_newBranch->isChecked();
}

void QGitWorktreeDialog::on_pushButton_browse_clicked()
{
    const QString repoAbs = m_git.path().absolutePath();
    const QString startDir = repoAbs.isEmpty()
        ? QDir::homePath()
        : QFileInfo(repoAbs).absolutePath();

    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Worktree Parent Directory"),
        startDir
    );
    if (!dir.isEmpty()) {
        const QString name = worktreeName();
        if (!name.isEmpty()) {
            ui->lineEdit_path->setText(QDir(dir).filePath(name));
        } else {
            ui->lineEdit_path->setText(dir);
            ui->lineEdit_name->setText(QDir(dir).dirName());
        }
    }
}

void QGitWorktreeDialog::on_lineEdit_name_textChanged(const QString &text)
{
    Q_UNUSED(text)
    validate();
}

void QGitWorktreeDialog::on_lineEdit_path_textChanged(const QString &text)
{
    Q_UNUSED(text)
    validate();
}

void QGitWorktreeDialog::on_checkBox_newBranch_toggled(bool checked)
{
    ui->comboBox_branch->setEnabled(!checked);
    validate();
}

void QGitWorktreeDialog::validate()
{
    const QString name = worktreeName();
    const QString path = worktreePath();
    const bool newBranch = createNewBranch();

    bool nameOk = !name.isEmpty();
    if (nameOk && m_existingWorktreeNames.contains(name)) {
        ui->lineEdit_name->setStyleSheet(errorInputStyle());
        ui->lineEdit_name->setToolTip(tr("A worktree named '%1' already exists.").arg(name));
        nameOk = false;
    } else if (nameOk && newBranch) {
        int refValid = 0;
        const QByteArray fullRef = (QStringLiteral("refs/heads/") + name).toUtf8();
        const bool validRef = (git_reference_name_is_valid(&refValid, fullRef.constData()) == 0) && (refValid != 0);
        if (!validRef) {
            ui->lineEdit_name->setStyleSheet(errorInputStyle());
            ui->lineEdit_name->setToolTip(tr("Invalid git branch name."));
            nameOk = false;
        } else if (m_existingBranchNames.contains(name)) {
            ui->lineEdit_name->setStyleSheet(errorInputStyle());
            ui->lineEdit_name->setToolTip(tr("Local branch '%1' already exists.").arg(name));
            nameOk = false;
        }
    }
    if (nameOk || name.isEmpty()) {
        ui->lineEdit_name->setStyleSheet(QString());
        ui->lineEdit_name->setToolTip(QString());
    }

    bool pathOk = !path.isEmpty();
    if (pathOk && QFileInfo::exists(path)) {
        ui->lineEdit_path->setStyleSheet(errorInputStyle());
        ui->lineEdit_path->setToolTip(tr("Target path already exists."));
        pathOk = false;
    } else {
        ui->lineEdit_path->setStyleSheet(QString());
        ui->lineEdit_path->setToolTip(QString());
    }

    const bool branchOk = newBranch || (ui->comboBox_branch->currentIndex() >= 0);

    if (auto *btn = ui->buttonBox->button(QDialogButtonBox::Ok))
        btn->setEnabled(nameOk && pathOk && branchOk);
}
