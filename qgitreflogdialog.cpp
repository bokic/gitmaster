#include "qgitreflogdialog.h"
#include "ui_qgitreflogdialog.h"
#include "qgit.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>

QGitReflogDialog::QGitReflogDialog(const QString &refName, QGitRepository *repository, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitReflogDialog)
    , m_refName(refName)
    , m_repository(repository)
{
    ui->setupUi(this);
    ui->label_info->setText(tr("Reflog for reference: %1").arg(refName));

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);

    loadReflog();
    on_tableWidget_itemSelectionChanged();
}

QGitReflogDialog::~QGitReflogDialog()
{
    delete ui;
}

void QGitReflogDialog::loadReflog()
{
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setSortingEnabled(false);

    QList<QGitReflogEntry> entries = m_repository->git()->getReflog(m_refName);
    for (const auto &entry : entries) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        auto *itemHash = new QTableWidgetItem(entry.commitHash.left(8));
        itemHash->setData(Qt::UserRole, entry.commitHash);
        
        QString committerStr = entry.committerName;
        if (!entry.committerEmail.isEmpty()) {
            committerStr += QStringLiteral(" <%1>").arg(entry.committerEmail);
        }
        auto *itemCommitter = new QTableWidgetItem(committerStr);
        
        QString dateStr = QDateTime::fromSecsSinceEpoch(entry.time).toString();
        auto *itemDate = new QTableWidgetItem(dateStr);
        
        auto *itemMsg = new QTableWidgetItem(entry.message);

        ui->tableWidget->setItem(row, 0, itemHash);
        ui->tableWidget->setItem(row, 1, itemCommitter);
        ui->tableWidget->setItem(row, 2, itemDate);
        ui->tableWidget->setItem(row, 3, itemMsg);
    }
}

QString QGitReflogDialog::selectedCommitHash() const
{
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        auto *item = ui->tableWidget->item(row, 0);
        if (item) {
            return item->data(Qt::UserRole).toString();
        }
    }
    return QString();
}

void QGitReflogDialog::on_tableWidget_itemSelectionChanged()
{
    bool hasSelection = !selectedCommitHash().isEmpty();
    ui->checkoutButton->setEnabled(hasSelection);
    ui->createBranchButton->setEnabled(hasSelection);
    ui->resetButton->setEnabled(hasSelection);
}

void QGitReflogDialog::on_checkoutButton_clicked()
{
    QString hash = selectedCommitHash();
    if (hash.isEmpty()) return;

    bool ok;
    QString branchName = QInputDialog::getText(this, tr("Checkout Commit"),
                                                 tr("Create and checkout a new branch at commit %1:").arg(hash.left(7)),
                                                 QLineEdit::Normal,
                                                 QStringLiteral("reflog-recover"),
                                                 &ok);
    if (ok && !branchName.isEmpty()) {
        try {
            m_repository->git()->createLocalBranch(branchName, hash, true);
            m_repository->refreshData();
            accept();
        } catch (const QGitError &error) {
            QMessageBox::warning(this, tr("Checkout Commit Error"), error.errorString());
        }
    }
}

void QGitReflogDialog::on_createBranchButton_clicked()
{
    QString hash = selectedCommitHash();
    if (hash.isEmpty()) return;

    bool ok;
    QString branchName = QInputDialog::getText(this, tr("Create Branch"),
                                                 tr("Create a new branch at commit %1:").arg(hash.left(7)),
                                                 QLineEdit::Normal,
                                                 QStringLiteral("reflog-branch"),
                                                 &ok);
    if (ok && !branchName.isEmpty()) {
        try {
            m_repository->git()->createLocalBranch(branchName, hash, false);
            m_repository->refreshData();
            loadReflog();
            QMessageBox::information(this, tr("Branch Created"), tr("Branch '%1' successfully created.").arg(branchName));
        } catch (const QGitError &error) {
            QMessageBox::warning(this, tr("Create Branch Error"), error.errorString());
        }
    }
}

void QGitReflogDialog::on_resetButton_clicked()
{
    QString hash = selectedCommitHash();
    if (hash.isEmpty()) return;

    git_reset_t resetType = GIT_RESET_SOFT;
    QString typeStr = tr("Soft");
    int index = ui->resetType_comboBox->currentIndex();
    if (index == 1) {
        resetType = GIT_RESET_MIXED;
        typeStr = tr("Mixed");
    } else if (index == 2) {
        resetType = GIT_RESET_HARD;
        typeStr = tr("Hard");
    }

    auto confirm = QMessageBox::question(this, tr("Reset Current Branch"),
                                         tr("Are you sure you want to perform a %1 Reset to commit %2?\n\nThis will move your current branch pointer.")
                                         .arg(typeStr, hash.left(7)),
                                         QMessageBox::Yes | QMessageBox::No);
    if (confirm == QMessageBox::Yes) {
        try {
            m_repository->git()->reset(hash, resetType);
            m_repository->refreshData();
            QMessageBox::information(this, tr("Reset Successful"), tr("Branch successfully reset to commit %1.").arg(hash.left(7)));
            accept();
        } catch (const QGitError &error) {
            QMessageBox::critical(this, tr("Reset Error"), error.errorString());
        }
    }
}
