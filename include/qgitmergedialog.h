#pragma once

#include <QDialog>
#include <QDir>
#include <QTableWidgetItem>
#include "qgit.h"
#include "qgitbranch.h"


namespace Ui {
class QGitMergeDialog;
}

class QGitMergeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitMergeDialog(QDir repoPath, QWidget *parent = nullptr);
    ~QGitMergeDialog();

    QString selectedBranch() const;
    bool commitImmediately() const;
    bool includeMessages() const;
    bool createCommitEvenIfFastForward() const;
    bool rebaseInsteadOfMerge() const;
    bool detectRenames() const;
    int renameSimilarity() const;
    void setRebaseMode(bool rebaseOnly);

private slots:
    void on_mergedFromLog_toolButton_clicked();
    void on_mergeFetched_toolButton_clicked();
    void on_checkBox_6_toggled(bool checked);
    void on_comboBox_2_currentIndexChanged(int index);
    void on_checkBox_toggled(bool checked);
    void on_lineEdit_textChanged(const QString &text);
    void updateButtonsState();

private:
    void populateBranches();
    void populateLogTable();

    Ui::QGitMergeDialog *ui;
    QDir m_repoPath;
    QGit m_git;
    QList<QGitBranch> m_allBranches;
};
