#include "qgitremotesdialog.h"
#include "ui_qgitremotesdialog.h"
#include "qgit.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QHeaderView>

QGitRemotesDialog::QGitRemotesDialog(QGitRepository *repository, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitRemotesDialog)
    , m_repository(repository)
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    loadRemotes();
    on_tableWidget_itemSelectionChanged();
}

QGitRemotesDialog::~QGitRemotesDialog()
{
    delete ui;
}

void QGitRemotesDialog::loadRemotes()
{
    QList<QGitRemote> remotes = m_repository->git()->remotes();
    ui->tableWidget->setRowCount(remotes.count());

    for (int row = 0; row < remotes.count(); ++row) {
        const auto &remote = remotes.at(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(remote.name));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(remote.url));
    }
}

QString QGitRemotesDialog::selectedRemoteName() const
{
    int row = ui->tableWidget->currentRow();
    if (row >= 0) {
        auto *item = ui->tableWidget->item(row, 0);
        if (item) return item->text();
    }
    return QString();
}

void QGitRemotesDialog::on_tableWidget_itemSelectionChanged()
{
    bool hasSelection = !selectedRemoteName().isEmpty();
    ui->editUrlButton->setEnabled(hasSelection);
    ui->renameButton->setEnabled(hasSelection);
    ui->deleteButton->setEnabled(hasSelection);
}

void QGitRemotesDialog::on_addButton_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Add Remote"),
                                          tr("Remote name:"), QLineEdit::Normal,
                                          QString(), &ok);
    if (!ok || name.trimmed().isEmpty()) return;

    QString url = QInputDialog::getText(this, tr("Add Remote"),
                                         tr("Remote URL:"), QLineEdit::Normal,
                                         QString(), &ok);
    if (!ok || url.trimmed().isEmpty()) return;

    try {
        m_repository->git()->addRemote(name.trimmed(), url.trimmed());
        m_repository->refreshData();
        loadRemotes();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Add Remote Failed"), error.errorString());
    }
}

void QGitRemotesDialog::on_editUrlButton_clicked()
{
    QString name = selectedRemoteName();
    if (name.isEmpty()) return;

    // Get current URL from row
    int row = ui->tableWidget->currentRow();
    QString currentUrl = ui->tableWidget->item(row, 1)->text();

    bool ok;
    QString newUrl = QInputDialog::getText(this, tr("Edit Remote URL"),
                                            tr("New fetch URL for '%1':").arg(name),
                                            QLineEdit::Normal, currentUrl, &ok);
    if (!ok || newUrl.trimmed().isEmpty()) return;

    try {
        m_repository->git()->setRemoteUrl(name, newUrl.trimmed(), false);
        m_repository->refreshData();
        loadRemotes();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Edit URL Failed"), error.errorString());
    }
}

void QGitRemotesDialog::on_renameButton_clicked()
{
    QString oldName = selectedRemoteName();
    if (oldName.isEmpty()) return;

    bool ok;
    QString newName = QInputDialog::getText(this, tr("Rename Remote"),
                                             tr("New name for remote '%1':").arg(oldName),
                                             QLineEdit::Normal, oldName, &ok);
    if (!ok || newName.trimmed().isEmpty() || newName.trimmed() == oldName) return;

    try {
        m_repository->git()->renameRemote(oldName, newName.trimmed());
        m_repository->refreshData();
        loadRemotes();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Rename Remote Failed"), error.errorString());
    }
}

void QGitRemotesDialog::on_deleteButton_clicked()
{
    QString name = selectedRemoteName();
    if (name.isEmpty()) return;

    auto res = QMessageBox::question(this, tr("Delete Remote"),
                                      tr("Are you sure you want to delete remote '%1'?").arg(name),
                                      QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes) return;

    try {
        m_repository->git()->deleteRemote(name);
        m_repository->refreshData();
        loadRemotes();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Delete Remote Failed"), error.errorString());
    }
}
