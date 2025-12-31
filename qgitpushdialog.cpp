#include "qgitpushdialog.h"
#include "ui_qgitpushdialog.h"


QGitPushDialog::QGitPushDialog(QGitRepository *parent)
    : QDialog(parent)
    , ui(new Ui::QGitPushDialog)
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

    ui->branches_tableWidget->setColumnCount(1);

    auto currentBranch = git->currentBranch();

    const auto localBranches = git->branches(GIT_BRANCH_LOCAL);
    for(const auto &localBranch: std::as_const(localBranches))
    {
        QTableWidgetItem *item = new QTableWidgetItem(localBranch.name());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (localBranch.name() == currentBranch)
        {
            QFont font = item->font();
            font.setBold(true);
            item->setFont(font);
            item->setCheckState(Qt::Checked);
        } else
        {
            item->setCheckState(Qt::Unchecked);
        }
        ui->branches_tableWidget->insertRow(ui->branches_tableWidget->rowCount());
        ui->branches_tableWidget->setItem(ui->branches_tableWidget->rowCount() - 1, 0, item);
    }
}

QGitPushDialog::~QGitPushDialog()
{
    delete ui;
}

QString QGitPushDialog::remote() const
{
    return ui->remote_comboBox->currentText();
}

QStringList QGitPushDialog::branches() const
{
    QStringList ret;

    for(int row = 0; row < ui->branches_tableWidget->rowCount(); row++)
    {
        auto item = ui->branches_tableWidget->item(row, 0);

        if (item->isSelected())
        {
            ret.append(item->text());
        }
    }

    return ret;
}

bool QGitPushDialog::tags() const
{
    return ui->pushAllTags_checkBox->isChecked();
}

bool QGitPushDialog::force() const
{
    return ui->forcePush_checkBox->isChecked();
}

void QGitPushDialog::on_selectAllBranches_checkBox_checkStateChanged(const Qt::CheckState &value)
{
    if (value == Qt::Checked)
    {
        m_savedBranches.clear();
        for(int row = 0; row < ui->branches_tableWidget->rowCount(); row++)
        {
            auto item = ui->branches_tableWidget->item(row, 0);

            if (item->isSelected() == false)
            {
                m_savedBranches.append(item->text());
                item->setSelected(true);
            }
        }

        ui->branches_tableWidget->setEnabled(false);
    }
    else
    {
        for(int row = 0; row < ui->branches_tableWidget->rowCount(); row++)
        {
            auto item = ui->branches_tableWidget->item(row, 0);

            if (m_savedBranches.contains(item->text()))
            {
                item->setSelected(false);
            }
        }
        m_savedBranches.clear();

        ui->branches_tableWidget->setEnabled(true);
    }
}

void QGitPushDialog::on_remote_comboBox_currentIndexChanged(int index)
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
