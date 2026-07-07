#include "qgitworktreedialog.h"
#include "ui_qgitworktreedialog.h"
#include "qgitrepository.h"
#include "qgit.h"

#include <QFileDialog>
#include <QDir>


QGitWorktreeDialog::QGitWorktreeDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitWorktreeDialog)
{
    ui->setupUi(this);

    m_git.setPath(parent->git()->path());

    // Populate branch combobox with local branches
    auto branches = m_git.branches(GIT_BRANCH_LOCAL);
    auto currentBranch = m_git.currentBranch();
    for (const auto &branch : branches) {
        // Extract short name from "refs/heads/..." format
        QString name = branch.name();
        if (name.startsWith(QStringLiteral("refs/heads/")))
            name = name.mid(11);
        ui->comboBox_branch->addItem(name);
    }

    // Pre-select current branch
    int idx = ui->comboBox_branch->findText(currentBranch);
    if (idx >= 0)
        ui->comboBox_branch->setCurrentIndex(idx);

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
    QString dir = QFileDialog::getExistingDirectory(
        this,
        tr("Select Worktree Directory"),
        m_git.path().absolutePath()
    );
    if (!dir.isEmpty()) {
        ui->lineEdit_path->setText(dir);
        // Auto-fill name from the last path component if name is empty
        if (ui->lineEdit_name->text().trimmed().isEmpty()) {
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
    bool ok = !ui->lineEdit_name->text().trimmed().isEmpty()
           && !ui->lineEdit_path->text().trimmed().isEmpty();

    if (auto *btn = ui->buttonBox->button(QDialogButtonBox::Ok))
        btn->setEnabled(ok);
}
