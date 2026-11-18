// SPDX-License-Identifier: LGPL-3.0-or-later
#include "qrepositorysettingsdialog.h"
#include "qgitmastermainwindow.h"
#include "ui_qrepositorysettingsdialog.h"
#include "qgit.h"

#include <QDir>
#include <QFileDialog>
#include <QPushButton>

QRepositorySettingsDialog::QRepositorySettingsDialog(const QString &name, const QString &path, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QRepositorySettingsDialog)
    , m_originalName(name)
{
    ui->setupUi(this);

    ui->lineEditName->setText(name);
    ui->lineEditPath->setText(path);

    connect(ui->lineEditPath, &QLineEdit::textChanged, this, &QRepositorySettingsDialog::updateWidgets);
    connect(ui->lineEditName, &QLineEdit::textChanged, this, &QRepositorySettingsDialog::updateWidgets);

    updateWidgets();
}

QRepositorySettingsDialog::~QRepositorySettingsDialog()
{
    delete ui;
}

QString QRepositorySettingsDialog::repositoryName() const
{
    return ui->lineEditName->text().trimmed();
}

QString QRepositorySettingsDialog::repositoryPath() const
{
    return QDir::toNativeSeparators(QDir::cleanPath(ui->lineEditPath->text().trimmed()));
}

void QRepositorySettingsDialog::updateWidgets()
{
    bool pathValid = true;
    bool nameValid = true;

    const QString pathText = ui->lineEditPath->text().trimmed();
    if (pathText.isEmpty()) {
        ui->lineEditPath->setStyleSheet("background: red");
        ui->lineEditPath->setToolTip(tr("Path is empty."));
        pathValid = false;
    } else {
        const QDir dir(pathText);
        if (!dir.exists()) {
            ui->lineEditPath->setStyleSheet("background: red");
            ui->lineEditPath->setToolTip(tr("Path doesn't exist."));
            pathValid = false;
        } else if (!QGit::isGitRepository(dir)) {
            ui->lineEditPath->setStyleSheet("background: red");
            ui->lineEditPath->setToolTip(tr("Dir is not valid git repository."));
            pathValid = false;
        } else {
            ui->lineEditPath->setStyleSheet("");
            ui->lineEditPath->setToolTip("");
        }
    }

    const QString nameText = ui->lineEditName->text().trimmed();
    const bool hasControlChars = [&nameText]() {
        for (const QChar &ch : nameText) {
            if (ch.isNull() || ch == '\n' || ch == '\r' || ch == '\t') {
                return true;
            }
        }
        return false;
    }();

    if (nameText.isEmpty() || hasControlChars) {
        ui->lineEditName->setStyleSheet("background: red");
        ui->lineEditName->setToolTip(nameText.isEmpty()
                                         ? tr("Bookmark name is empty.")
                                         : tr("Bookmark name contains invalid characters."));
        nameValid = false;
    } else {
        auto mainWindow = dynamic_cast<QGitMasterMainWindow *>(parent());
        if (mainWindow && nameText != m_originalName && mainWindow->hasRepositoryWithName(nameText)) {
            ui->lineEditName->setStyleSheet("background: red");
            ui->lineEditName->setToolTip(tr("Bookmark name already exist. Please choose another name."));
            nameValid = false;
        } else {
            ui->lineEditName->setStyleSheet("");
            ui->lineEditName->setToolTip("");
        }
    }

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(pathValid && nameValid);
}

void QRepositorySettingsDialog::on_toolButtonBrowse_clicked()
{
    const QString currentPath = ui->lineEditPath->text().trimmed();
    const QString startDir = QDir(currentPath).exists() ? currentPath : QString();
    const QString dirStr = QFileDialog::getExistingDirectory(this, tr("Select Repository Directory"), startDir);

    if (!dirStr.isEmpty()) {
        ui->lineEditPath->setText(QDir::toNativeSeparators(dirStr));
        if (ui->lineEditName->text().trimmed().isEmpty()) {
            ui->lineEditName->setText(QDir(dirStr).dirName());
        }
    }

    updateWidgets();
}
