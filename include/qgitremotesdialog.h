#pragma once

#include <QDialog>
#include "qgitrepository.h"

namespace Ui {
class QGitRemotesDialog;
}

class QGitRemotesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitRemotesDialog(QGitRepository *repository, QWidget *parent = nullptr);
    ~QGitRemotesDialog();

private slots:
    void on_addButton_clicked();
    void on_editUrlButton_clicked();
    void on_renameButton_clicked();
    void on_deleteButton_clicked();
    void on_tableWidget_itemSelectionChanged();

private:
    void loadRemotes();
    QString selectedRemoteName() const;

    Ui::QGitRemotesDialog *ui = nullptr;
    QGitRepository *m_repository = nullptr;
};
