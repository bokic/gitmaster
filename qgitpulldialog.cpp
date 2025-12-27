#include "qgitpulldialog.h"
#include "ui_qgitpulldialog.h"
#include "qgitrepository.h"
#include "qgit.h"

#include <QStandardItemModel>
#include <QDialog>


QGitPullDialog::QGitPullDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitPullDialog)
{
    ui->setupUi(this);

    QGit *git = static_cast<QGitRepository *>(parent)->git();

    auto remotes = git->remotes();
    for(const auto &remote: std::as_const(remotes))
    {
        const auto name = remote.name;
        const auto url = remote.url;

        ui->remote_comboBox->addItem(name, url);
    }
    ui->remote_comboBox->addItem(tr("custom"));

    auto branches = git->localBranches();
    auto currentBranch = git->localBranch();

    auto branch_comboBox_model = static_cast<QStandardItemModel *>(ui->branch_comboBox->model());

    for(const auto &branch: std::as_const(branches))
    {
        QStandardItem *item = new QStandardItem(branch);

        if (branch == currentBranch)
        {
            auto font = item->font();
            font.setBold(true);
            item->setFont(font);
        }

        branch_comboBox_model->appendRow(item);
    }

    ui->branch_comboBox->setCurrentText(currentBranch);
}

QGitPullDialog::~QGitPullDialog()
{
    delete ui;
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
}
