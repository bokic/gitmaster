#include "qgitpulldialog.h"
#include "ui_qgitpulldialog.h"
#include "qgitrepository.h"
#include "qgit.h"

#include <QStandardItemModel>
#include <QDialog>


QGitPullDialog::QGitPullDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitPullDialog)
    , m_repository(parent)
{
    ui->setupUi(this);

    if (m_repository && m_repository->git())
    {
        QGit *git = m_repository->git();

        auto remotes = git->remotes();
        for (const auto &remote : std::as_const(remotes))
        {
            ui->remote_comboBox->addItem(remote.name, remote.url);
        }
        ui->remote_comboBox->addItem(tr("custom"));

        ui->label_4->setText(git->currentBranch());
        ui->branch_comboBox->setEditable(true);

        loadBranches();
    }
}

QGitPullDialog::~QGitPullDialog()
{
    delete ui;
}

void QGitPullDialog::loadBranches()
{
    if (!m_repository || !m_repository->git()) return;

    QString currentSelected = ui->branch_comboBox->currentText();
    ui->branch_comboBox->clear();

    QGit *git = m_repository->git();
    QString selectedRemote = ui->remote_comboBox->currentText();
    QString remotePrefix = selectedRemote + "/";

    auto branches = git->branches(GIT_BRANCH_REMOTE);
    auto currentLocalBranch = git->currentBranch();

    auto branch_comboBox_model = qobject_cast<QStandardItemModel *>(ui->branch_comboBox->model());
    QString defaultBranch;

    for (const auto &branch : std::as_const(branches))
    {
        QString branchName = branch.name();
        if (selectedRemote != tr("custom") && !selectedRemote.isEmpty())
        {
            if (branchName.startsWith(remotePrefix))
            {
                QString shortName = branchName.mid(remotePrefix.length());
                if (shortName == QStringLiteral("HEAD")) continue;

                QStandardItem *item = new QStandardItem(shortName);
                if (shortName == currentLocalBranch)
                {
                    auto font = item->font();
                    font.setBold(true);
                    item->setFont(font);
                    defaultBranch = shortName;
                }
                if (branch_comboBox_model)
                    branch_comboBox_model->appendRow(item);
                else
                    ui->branch_comboBox->addItem(shortName);
            }
        }
        else
        {
            QStandardItem *item = new QStandardItem(branchName);
            if (branch_comboBox_model)
                branch_comboBox_model->appendRow(item);
            else
                ui->branch_comboBox->addItem(branchName);
        }
    }

    if (ui->branch_comboBox->count() == 0)
    {
        auto localBranches = git->branches(GIT_BRANCH_LOCAL);
        for (const auto &branch : std::as_const(localBranches))
        {
            QStandardItem *item = new QStandardItem(branch.name());
            if (branch.name() == currentLocalBranch)
            {
                auto font = item->font();
                font.setBold(true);
                item->setFont(font);
                defaultBranch = branch.name();
            }
            if (branch_comboBox_model)
                branch_comboBox_model->appendRow(item);
            else
                ui->branch_comboBox->addItem(branch.name());
        }
    }

    if (!currentSelected.isEmpty() && ui->branch_comboBox->findText(currentSelected) >= 0)
    {
        ui->branch_comboBox->setCurrentText(currentSelected);
    }
    else if (!defaultBranch.isEmpty())
    {
        ui->branch_comboBox->setCurrentText(defaultBranch);
    }
    else if (ui->branch_comboBox->findText(currentLocalBranch) >= 0)
    {
        ui->branch_comboBox->setCurrentText(currentLocalBranch);
    }
}

QGitRemote QGitPullDialog::remote() const
{
    QGitRemote ret;

    ret.name = ui->remote_comboBox->currentText();
    ret.url = ui->url_lineEdit->text();

    return ret;
}

QString QGitPullDialog::branch() const
{
    return ui->branch_comboBox->currentText();
}

bool QGitPullDialog::commitMergedChanges() const
{
    return ui->commit_merged_checkBox->isChecked();
}

bool QGitPullDialog::includeMessages() const
{
    return ui->include_messages_checkBox->isChecked();
}

bool QGitPullDialog::createNewCommit() const
{
    return ui->create_new_commit_checkBox->isChecked();
}

bool QGitPullDialog::rebase() const
{
    return ui->rebase_checkBox->isChecked();
}

void QGitPullDialog::on_remote_comboBox_currentIndexChanged(int index)
{
    QString url;

    if (index >= 0)
    {
        url = ui->remote_comboBox->itemData(index).toString();
        ui->url_lineEdit->setText(url);
    }

    if (!url.isEmpty())
    {
        ui->url_lineEdit->setEnabled(false);
    }
    else
    {
        ui->url_lineEdit->setEnabled(true);
        ui->url_lineEdit->setFocus();
    }

    loadBranches();
}

void QGitPullDialog::on_pushButton_clicked()
{
    loadBranches();
}
