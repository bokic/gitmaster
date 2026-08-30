#include "qgitfetchdialog.h"
#include "ui_qgitfetchdialog.h"
#include "qgitrepository.h"
#include "qgit.h"


QGitFetchDialog::QGitFetchDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitFetchDialog)
{
    ui->setupUi(this);

    auto repo = qobject_cast<QGitRepository *>(parent);
    if (repo && repo->git())
    {
        auto remotes = repo->git()->remotes();
        for (const auto &remote : std::as_const(remotes))
        {
            ui->comboBox_remote->addItem(remote.name, remote.url);
        }
    }

    bool fetchAll = ui->checkBox_fetchFromAllRemotes->isChecked();
    ui->comboBox_remote->setEnabled(!fetchAll);
    ui->label_remote->setEnabled(!fetchAll);
}

QGitFetchDialog::~QGitFetchDialog()
{
    delete ui;
}

QString QGitFetchDialog::remote() const
{
    return ui->comboBox_remote->currentText();
}

bool QGitFetchDialog::fetchFromAllRemotes() const
{
    return ui->checkBox_fetchFromAllRemotes->isChecked();
}

bool QGitFetchDialog::purgeDeletedBranches() const
{
    return ui->checkBox_PurgeDeletedBranches->isChecked();
}

bool QGitFetchDialog::fetchAllTags() const
{
    return ui->checkBox_fetchAllTags->isChecked();
}

void QGitFetchDialog::on_checkBox_fetchFromAllRemotes_toggled(bool checked)
{
    ui->comboBox_remote->setEnabled(!checked);
    ui->label_remote->setEnabled(!checked);
}
