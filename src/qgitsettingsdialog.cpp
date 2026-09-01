#include "qgitsettingsdialog.h"
#include "ui_qgitsettingsdialog.h"
#include <QInputDialog>
#include <QMessageBox>

QGitSettingsDialog::QGitSettingsDialog(QGit *git, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QGitSettingsDialog)
    , m_git(git)
{
    ui->setupUi(this);

    bool isRepo = QGit::isGitRepository(m_git->path());
    if (!isRepo) {
        ui->radioButton_local->setEnabled(false);
        ui->radioButton_global->setChecked(true);
    }

    // Setup Advanced Table properties
    ui->tableWidget->setColumnCount(2);
    QStringList headers;
    headers << tr("Key") << tr("Value");
    ui->tableWidget->setHorizontalHeaderLabels(headers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Load configurations
    loadSettings();
    try {
        m_allConfigs = m_git->configEntries();
    } catch (const QGitError &) {
        // Safe fallback if open fails
    }
    populateTable();
}

QGitSettingsDialog::~QGitSettingsDialog()
{
    delete ui;
}

void QGitSettingsDialog::loadSettings()
{
    try {
        ui->lineEdit_userName->setText(m_git->configString("user.name"));
        ui->lineEdit_userEmail->setText(m_git->configString("user.email"));
        QString autocrlf = m_git->configString("core.autocrlf");
        if (autocrlf.isEmpty()) autocrlf = "false";
        int index = ui->comboBox_autoCrlf->findText(autocrlf);
        if (index >= 0) ui->comboBox_autoCrlf->setCurrentIndex(index);
    } catch (const QGitError &error) {
        // Ignore load errors for fallback
    }
}

void QGitSettingsDialog::populateTable()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    QString filter = ui->lineEdit_search->text().trimmed();
    int row = 0;

    QMapIterator<QString, QString> it(m_allConfigs);
    while (it.hasNext()) {
        it.next();
        if (!filter.isEmpty() && 
            !it.key().contains(filter, Qt::CaseInsensitive) && 
            !it.value().contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        ui->tableWidget->insertRow(row);

        QTableWidgetItem *keyItem = new QTableWidgetItem(it.key());
        QTableWidgetItem *valueItem = new QTableWidgetItem(it.value());

        keyItem->setFlags(keyItem->flags() & ~Qt::ItemIsEditable);
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);

        ui->tableWidget->setItem(row, 0, keyItem);
        ui->tableWidget->setItem(row, 1, valueItem);
        row++;
    }
}

void QGitSettingsDialog::on_pushButton_add_clicked()
{
    bool ok;
    QString key = QInputDialog::getText(this, tr("Add Configuration"), tr("Key (e.g., core.autocrlf):"), QLineEdit::Normal, "", &ok);
    if (!ok || key.trimmed().isEmpty()) return;

    QString val = QInputDialog::getText(this, tr("Add Configuration"), tr("Value:"), QLineEdit::Normal, "", &ok);
    if (!ok) return;

    bool globalWrite = ui->radioButton_global->isChecked();
    try {
        m_git->setConfigString(key.trimmed(), val, globalWrite);
        m_allConfigs = m_git->configEntries();
        populateTable();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}

void QGitSettingsDialog::on_pushButton_edit_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, tr("Edit Configuration"), tr("Please select a configuration key to edit."));
        return;
    }
    QString key = ui->tableWidget->item(row, 0)->text();
    QString currentVal = ui->tableWidget->item(row, 1)->text();

    bool ok;
    QString val = QInputDialog::getText(this, tr("Edit Configuration"), tr("Value for '%1':").arg(key), QLineEdit::Normal, currentVal, &ok);
    if (!ok) return;

    bool globalWrite = ui->radioButton_global->isChecked();
    try {
        m_git->setConfigString(key, val, globalWrite);
        m_allConfigs = m_git->configEntries();
        populateTable();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}

void QGitSettingsDialog::on_pushButton_delete_clicked()
{
    int row = ui->tableWidget->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, tr("Delete Configuration"), tr("Please select a configuration key to delete."));
        return;
    }
    QString key = ui->tableWidget->item(row, 0)->text();

    auto confirm = QMessageBox::question(this, tr("Delete Configuration"),
                                         tr("Are you sure you want to delete the configuration key '%1'?").arg(key),
                                         QMessageBox::Yes | QMessageBox::No);
    if (confirm != QMessageBox::Yes) return;

    bool globalWrite = ui->radioButton_global->isChecked();
    try {
        m_git->deleteConfigEntry(key, globalWrite);
        m_allConfigs = m_git->configEntries();
        populateTable();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}

void QGitSettingsDialog::on_lineEdit_search_textChanged(const QString &text)
{
    Q_UNUSED(text);
    populateTable();
}

void QGitSettingsDialog::on_radioButton_local_toggled(bool checked)
{
    Q_UNUSED(checked);
}

void QGitSettingsDialog::on_radioButton_global_toggled(bool checked)
{
    Q_UNUSED(checked);
}

void QGitSettingsDialog::accept()
{
    QString name = ui->lineEdit_userName->text().trimmed();
    QString email = ui->lineEdit_userEmail->text().trimmed();
    QString autocrlf = ui->comboBox_autoCrlf->currentText();

    bool globalWrite = ui->radioButton_global->isChecked();

    try {
        m_git->setConfigString("user.name", name, globalWrite);
        m_git->setConfigString("user.email", email, globalWrite);
        m_git->setConfigString("core.autocrlf", autocrlf, globalWrite);
        QDialog::accept();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Save Error"), error.errorString());
    }
}
