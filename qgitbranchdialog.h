#ifndef QGITBRANCHDIALOG_H
#define QGITBRANCHDIALOG_H

#include <qgitrepository.h>
#include <qgitbranch.h>
#include <qgit.h>

#include <QTableWidgetItem>
#include <QDialog>

namespace Ui {
class QGitBranchDialog;
}

class QGitBranchDialog : public QDialog
{
    Q_OBJECT

public:
    enum Operation {NewBranchOperation, DeleteBranchesOperation, UnknownOperation};

    explicit QGitBranchDialog(QGitRepository *parent = nullptr);
    ~QGitBranchDialog();

    Operation operation() const;
    QString newBranchName() const;
    QString newBranchCommitId() const;
    bool newBranchCheckout() const;
    QList<QGitBranch> deleteBranches() const;
    bool forceDelete() const;

private slots:
    void on_newBranch_pushButton_clicked();
    void on_deleteBranch_pushButton_clicked();
    void on_newBranch_lineEdit_textChanged(const QString &text);
    void on_newBranchCommitId_lineEdit_textChanged(const QString &text);
    void on_deleteBranches_tableWidget_itemChanged(QTableWidgetItem *item);

private:
    void checkForNewBranch();

    Ui::QGitBranchDialog *ui;
    QList<QGitBranch> m_currentBranches;
    QGit m_git;
};

#endif // QGITBRANCHDIALOG_H
