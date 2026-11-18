// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QDialog>

namespace Ui {
class QRepositorySettingsDialog;
}

class QRepositorySettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QRepositorySettingsDialog(const QString &name, const QString &path, QWidget *parent = nullptr);
    ~QRepositorySettingsDialog() override;

    QString repositoryName() const;
    QString repositoryPath() const;

private slots:
    void updateWidgets();
    void on_toolButtonBrowse_clicked();

private:
    Ui::QRepositorySettingsDialog *ui = nullptr;
    QString m_originalName;
};
