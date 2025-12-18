#include "qgitpushdialog.h"
#include "ui_qgitpushdialog.h"


QGitPushDialog::QGitPushDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitPushDialog)
{
    ui->setupUi(this);

    QGit *git = static_cast<QGitRepository *>(parent)->git();

    auto remotes = git->remotes();
    ui->remote_comboBox->addItems(remotes);

    ui->branches_tableWidget->setColumnCount(1);

    const auto localBranches = git->localBranches();
    for(const auto &localBranch: std::as_const(localBranches))
    {
        QTableWidgetItem *item = new QTableWidgetItem(localBranch);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        ui->branches_tableWidget->insertRow(ui->branches_tableWidget->rowCount());
        ui->branches_tableWidget->setItem(ui->branches_tableWidget->rowCount() - 1, 0, item);
    }
}

QGitPushDialog::~QGitPushDialog()
{
    delete ui;
}
