// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

#include <QDialog>
#include <QDir>
#include <QThread>
#include "qgit.h"

class QDialogButtonBox;
class QLineEdit;
class QLogHistoryTableWidget;

class QGitCommitPickerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QGitCommitPickerDialog(const QDir &repoPath, QWidget *parent = nullptr);
    ~QGitCommitPickerDialog() override;

    QString selectedCommitId() const;

signals:
    void requestCommits(const QString &branchRef, int offset, int length);

private slots:
    void onCommitsReply(const QList<QGitCommit> &commits, const QGitError &error);
    void onFilterTextChanged(const QString &text);
    void onCurrentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void onCellDoubleClicked(int row, int column);
    void onScrollValueChanged(int value);

private:
    void fetchMoreCommits();

    QDir m_repoPath;
    QThread m_thread;
    QGit *m_git = nullptr;
    QLineEdit *m_filterEdit = nullptr;
    QLogHistoryTableWidget *m_commitTable = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
    int m_offset = 0;
    bool m_loading = false;
    bool m_allLoaded = false;
};
