#pragma once

#include <QDialog>
#include "qgitrepository.h"


namespace Ui {
class QGitReflogDialog;
}

class QGitReflogDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitReflogDialog(const QString &refName, QGitRepository *repository, QWidget *parent = nullptr);
    ~QGitReflogDialog();

private slots:
    void on_checkoutButton_clicked();
    void on_createBranchButton_clicked();
    void on_resetButton_clicked();
    void on_tableWidget_itemSelectionChanged();

private:
    void loadReflog();
    QString selectedCommitHash() const;

    Ui::QGitReflogDialog *ui = nullptr;
    QString m_refName;
    QGitRepository *m_repository = nullptr;
};
