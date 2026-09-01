#pragma once

#include <QDialog>
#include <QString>
#include <QDir>
#include <QList>
#include "qgit.h"

namespace Ui {
class QGitSubmodulePointerDialog;
}

class QGitSubmodulePointerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitSubmodulePointerDialog(
        const QDir &repoPath,
        const QGitSubmodule &submodule,
        QWidget *parent = nullptr);
    ~QGitSubmodulePointerDialog() override;

    QString selectedTarget() const;
    QString trackedBranch() const;
    bool shouldUpdateTrackedBranch() const;

private slots:
    void on_tabWidget_currentChanged(int index);
    void on_branchesTable_itemSelectionChanged();
    void on_tagsTable_itemSelectionChanged();
    void on_commitsTable_itemSelectionChanged();
    void on_lineEdit_customCommit_textChanged(const QString &text);
    void on_lineEdit_filterBranches_textChanged(const QString &text);
    void on_lineEdit_filterTags_textChanged(const QString &text);
    void on_lineEdit_filterCommits_textChanged(const QString &text);
    void on_checkBox_updateTrackedBranch_toggled(bool checked);
    void updateButtons();

private:
    void loadSubmoduleData();
    void populateBranchesTable();
    void populateTagsTable();
    void populateCommitsTable();

    Ui::QGitSubmodulePointerDialog *ui;
    QDir m_repoPath;
    QGitSubmodule m_submodule;
    QList<QGitBranch> m_branches;
    QList<QGitTag> m_tags;
    QList<QGitCommit> m_commits;
    bool m_isInitialized = false;
};
