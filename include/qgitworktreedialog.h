#pragma once

#include <QDialog>
#include "qgit.h"

namespace Ui {
class QGitWorktreeDialog;
}

class QGitRepository;

class QGitWorktreeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitWorktreeDialog(QGitRepository *parent = nullptr);
    ~QGitWorktreeDialog();

    QString worktreeName() const;
    QString worktreePath() const;
    QString worktreeBranch() const;
    bool    createNewBranch() const;

private slots:
    void on_pushButton_browse_clicked();
    void on_lineEdit_name_textChanged(const QString &text);
    void on_lineEdit_path_textChanged(const QString &text);
    void on_checkBox_newBranch_toggled(bool checked);

private:
    void validate();

    Ui::QGitWorktreeDialog *ui;
    QGit m_git;
};
