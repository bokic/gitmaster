#pragma once

#include <QDialog>
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include "qgit.h"

namespace Ui {
class QGitBlameDialog;
}

class QGitBlameDialog : public QDialog
{
    Q_OBJECT

public:
    // Opens the blame browser rooted at the repository's working directory
    explicit QGitBlameDialog(QGit *git, QWidget *parent = nullptr);
    ~QGitBlameDialog();

    // Pre-load blame for a specific file (path relative to repo root),
    // optionally pinned to a particular commit. Called when the dialog is
    // opened directly from a commit's file list.
    void preSelectFile(const QString &relPath, const QString &commitId = QString());

signals:
    // Emitted when the user double-clicks a blame row — lets the caller
    // navigate to the commit in the log history view
    void commitNavigationRequested(const QString &commitHash);

private slots:
    void on_fileTree_itemClicked(QTreeWidgetItem *item, int column);
    void on_lineEdit_filter_textChanged(const QString &text);
    void on_blameTable_itemDoubleClicked(QTableWidgetItem *item);

private:
    // Build the file tree from the git working directory
    void populateFileTree(const QString &rootPath);
    void addDirectoryToTree(QTreeWidgetItem *parent,
                            const QString &dirPath,
                            const QString &repoRoot);
    void filterTree(QTreeWidgetItem *item, const QString &text);

    // Load and display blame for a file (path relative to repo root)
    void showBlameForFile(const QString &relPath);

    Ui::QGitBlameDialog *ui = nullptr;
    QGit *m_git = nullptr;
    QString m_pinnedCommitId; // when set, blame is shown as-of this commit

    // Palette of pastel background colours rotated per unique commit hash
    static QColor hunkColor(const QString &hash, int index);
};
