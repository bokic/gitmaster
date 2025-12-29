#ifndef QGITBRANCHDIALOG_H
#define QGITBRANCHDIALOG_H

#include <QDialog>

namespace Ui {
class QGitBranchDialog;
}

class QGitBranchDialog : public QDialog
{
    Q_OBJECT

public:
    enum Operation {NewBranchOperation, DeleteBranchesOperation, UnknownOperation};

    explicit QGitBranchDialog(QWidget *parent = nullptr);
    ~QGitBranchDialog();

    Operation operation() const;
    QString newBranchName() const;
    QString newBranchCommitId() const;
    bool newBranchCheckout() const;

private slots:
    void on_newBranch_pushButton_clicked();
    void on_deleteBranch_pushButton_clicked();
    void on_newBranch_lineEdit_textChanged(const QString &text);
    void on_newBranchCommitId_lineEdit_textChanged(const QString &text);

private:
    void checkForNewBranch();

    Ui::QGitBranchDialog *ui;
    QStringList m_currentBranches;
};

#endif // QGITBRANCHDIALOG_H
