#pragma once

#include "qcustomcombobox.h"
#include <QWidget>


class QComboBoxGitStatusFiles : public QCustomComboBox
{
    Q_OBJECT
public:
    enum QComboBoxGitStatusFilesShowFiles {QShowPendingFiles, QShowConflictFiles, QShowUntracked, QShowIgnored, QShowClean, QShowModified, QShowAll};
    enum QComboBoxGitStatusFilesOrderFiles {QUnsortedFiles, QFilePathSortFiles, QReversedFilePathSortFiles, QFileNameSortFiles, QReversedFileNameSortFiles, QFileStatusSortFiles, QCheckedUncheckedSortFiles};
    explicit QComboBoxGitStatusFiles(QWidget *parent = nullptr);
    void updateText();
    QComboBoxGitStatusFilesShowFiles showFiles() const;
    QComboBoxGitStatusFilesOrderFiles showSortBy() const;

protected:
    virtual void clicked(QListWidgetItem *item) override;
};
