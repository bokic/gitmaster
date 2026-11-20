// SPDX-License-Identifier: LGPL-3.0-or-later
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

    bool isRepo = false;
    bool hasLocal = false;
    bool hasGlobal = false;
    if (m_git) {
        QGit::invokeSync(m_git, [&]() {
            isRepo = QGit::isGitRepository(m_git->path());
            auto hasAnyGeneralSetting = [&](bool global) {
                try {
                    return !m_git->configString("user.name", global).isEmpty()
                        || !m_git->configString("user.email", global).isEmpty()
                        || !m_git->configString("core.autocrlf", global).isEmpty();
                } catch (const QGitError &) {
                    return false;
                }
            };
            if (isRepo) {
                hasLocal = hasAnyGeneralSetting(false);
            }
            hasGlobal = hasAnyGeneralSetting(true);
        });
    }

    {
        const QSignalBlocker blockLocal(ui->radioButton_local);
        const QSignalBlocker blockGlobal(ui->radioButton_global);
        if (!isRepo) {
            ui->radioButton_local->setEnabled(false);
            ui->radioButton_global->setChecked(true);
        } else if (hasLocal && !hasGlobal) {
            ui->radioButton_local->setChecked(true);
        } else {
            ui->radioButton_global->setChecked(true);
        }
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

    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, [this](int, int) {
        on_pushButton_edit_clicked();
    });

    // Load configurations
    reloadScope(ui->radioButton_global->isChecked());
}

QGitSettingsDialog::~QGitSettingsDialog()
{
    delete ui;
}

void QGitSettingsDialog::reloadScope(bool global)
{
    if (!m_git) {
        m_allConfigs.clear();
        populateTable();
        return;
    }

    try {
        QString userName, userEmail, autocrlf;
        QGit::invokeSync(m_git, [&]() {
            userName = m_git->configString("user.name", global);
            userEmail = m_git->configString("user.email", global);
            autocrlf = m_git->configString("core.autocrlf", global);
            m_allConfigs = m_git->configEntries(global);
        });
        ui->lineEdit_userName->setText(userName);
        ui->lineEdit_userEmail->setText(userEmail);
        int index = ui->comboBox_autoCrlf->findText(autocrlf);
        ui->comboBox_autoCrlf->setCurrentIndex(index >= 0 ? index : 0);
    } catch (const QGitError &error) {
        m_allConfigs.clear();
        qWarning("QGitSettingsDialog::reloadScope: %s", qPrintable(error.errorString()));
    }
    populateTable();
}

void QGitSettingsDialog::populateTable()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    QString filter = ui->lineEdit_search->text().trimmed();
    int row = 0;

    for (auto it = m_allConfigs.cbegin(); it != m_allConfigs.cend(); ++it) {
        if (!filter.isEmpty() && 
            !it.key().contains(filter, Qt::CaseInsensitive) && 
            !it.value().contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        ui->tableWidget->insertRow(row);

        QTableWidgetItem *keyItem = new QTableWidgetItem(it.key());
        QTableWidgetItem *valueItem = new QTableWidgetItem(it.value());

        keyItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        ui->tableWidget->setItem(row, 0, keyItem);
        ui->tableWidget->setItem(row, 1, valueItem);
        row++;
    }
}

void QGitSettingsDialog::on_pushButton_add_clicked()
{
    if (!m_git) return;

    bool ok;
    QString key = QInputDialog::getText(this, tr("Add Configuration"), tr("Key (e.g., core.autocrlf):"), QLineEdit::Normal, "", &ok);
    if (!ok || key.trimmed().isEmpty()) return;

    QString val = QInputDialog::getText(this, tr("Add Configuration"), tr("Value:"), QLineEdit::Normal, "", &ok);
    if (!ok) return;

    bool globalWrite = ui->radioButton_global->isChecked();
    try {
        QGit::invokeSync(m_git, [&]() {
            m_git->setConfigString(key.trimmed(), val, globalWrite);
        });
        m_configModified = true;
        reloadScope(globalWrite);
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}

void QGitSettingsDialog::on_pushButton_edit_clicked()
{
    if (!m_git) return;

    int row = ui->tableWidget->currentRow();
    const QTableWidgetItem *keyItem = (row >= 0) ? ui->tableWidget->item(row, 0) : nullptr;
    const QTableWidgetItem *valItem = (row >= 0) ? ui->tableWidget->item(row, 1) : nullptr;
    if (!keyItem || !valItem) {
        QMessageBox::warning(this, tr("Edit Configuration"), tr("Please select a configuration key to edit."));
        return;
    }
    QString key = keyItem->text();
    QString currentVal = valItem->text();

    bool ok;
    QString val = QInputDialog::getText(this, tr("Edit Configuration"), tr("Value for '%1':").arg(key), QLineEdit::Normal, currentVal, &ok);
    if (!ok) return;

    bool globalWrite = ui->radioButton_global->isChecked();
    try {
        QGit::invokeSync(m_git, [&]() {
            m_git->setConfigString(key, val, globalWrite);
        });
        m_configModified = true;
        reloadScope(globalWrite);
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}

void QGitSettingsDialog::on_pushButton_delete_clicked()
{
    if (!m_git) return;

    int row = ui->tableWidget->currentRow();
    const QTableWidgetItem *keyItem = (row >= 0) ? ui->tableWidget->item(row, 0) : nullptr;
    if (!keyItem) {
        QMessageBox::warning(this, tr("Delete Configuration"), tr("Please select a configuration key to delete."));
        return;
    }
    QString key = keyItem->text();

    auto confirm = QMessageBox::question(this, tr("Delete Configuration"),
                                         tr("Are you sure you want to delete the configuration key '%1'?").arg(key),
                                         QMessageBox::Yes | QMessageBox::No);
    if (confirm != QMessageBox::Yes) return;

    bool globalWrite = ui->radioButton_global->isChecked();
    try {
        QGit::invokeSync(m_git, [&]() {
            m_git->deleteConfigEntry(key, globalWrite);
        });
        m_configModified = true;
        reloadScope(globalWrite);
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
    if (!checked) return;
    reloadScope(false);
}

void QGitSettingsDialog::on_radioButton_global_toggled(bool checked)
{
    if (!checked) return;
    reloadScope(true);
}

void QGitSettingsDialog::accept()
{
    if (!m_git) {
        QDialog::accept();
        return;
    }

    QString name = ui->lineEdit_userName->text().trimmed();
    QString email = ui->lineEdit_userEmail->text().trimmed();
    QString autocrlf = ui->comboBox_autoCrlf->currentText();

    bool globalWrite = ui->radioButton_global->isChecked();

    try {
        QGit::invokeSync(m_git, [&]() {
            auto applyConfig = [&](const QString &key, const QString &value) {
                if (value.isEmpty()) {
                    m_git->deleteConfigEntry(key, globalWrite);
                } else {
                    m_git->setConfigString(key, value, globalWrite);
                }
            };
            applyConfig("user.name", name);
            applyConfig("user.email", email);
            applyConfig("core.autocrlf", autocrlf);
        });
        m_configModified = true;
        QDialog::accept();
    } catch (const QGitError &error) {
        QMessageBox::critical(this, tr("Save Error"), error.errorString());
    }
}
