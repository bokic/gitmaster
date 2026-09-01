#pragma once

#include <QDialog>
#include <QList>
#include <QSet>
#include "qgit.h"

namespace Ui {
class QGitStashInspectDialog;
}

class QGitStashInspectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitStashInspectDialog(QGit *git, const QString &stashRef, const QString &stashName, QWidget *parent = nullptr);
    ~QGitStashInspectDialog() override;

private slots:
    void on_tableWidget_files_itemSelectionChanged();
    void on_comboBox_gitDiffOptions_optionsChanged();
    void on_logHistory_commits_itemSelectionChanged();
    void onCommitDiffReply(const QString &commit_id, const QGitCommit &diff, const QGitError &error);
    void onCommitDiffContentReply(const QString &first, const QString &second, const QList<QGitDiffFile> &files, const QGitError &error);
    void onListCommitsReply(const QList<QGitCommit> &commits, const QGitError &error);

private:
    void loadStashDetails();
    void updateDiffForCurrentFile();

    Ui::QGitStashInspectDialog *ui = nullptr;
    QGit *m_git = nullptr;
    QString m_stashRef;
    QString m_stashName;
    QGitCommit m_stashCommit;
    QGitCommit m_activeCommitDiff;
    QSet<QString> m_parentHashes;
    QMap<QString, QGitCommit> m_commitMap;
};
