#pragma once

#include "qcomboboxgitbase.h"
#include <QString>

class QComboBoxGitStatusFiles : public QComboBoxGitBase
{
    Q_OBJECT
public:
    enum QComboBoxGitStatusFilesShowFiles {QShowPendingFiles, QShowConflictFiles, QShowUntracked, QShowIgnored, QShowClean, QShowModified, QShowAll};
    enum QComboBoxGitStatusFilesOrderFiles {QUnsortedFiles, QFilePathSortFiles, QReversedFilePathSortFiles, QFileNameSortFiles, QReversedFileNameSortFiles, QFileStatusSortFiles, QCheckedUncheckedSortFiles};

    explicit QComboBoxGitStatusFiles(QWidget *parent = nullptr);
    ~QComboBoxGitStatusFiles() override = default;

    void updateText();
    QComboBoxGitStatusFilesShowFiles showFiles() const;
    QComboBoxGitStatusFilesOrderFiles showSortBy() const;

signals:
    void itemClicked(int index);

protected:
    void updateIconColor() override;

private Q_SLOTS:
    void activated(int index);

private:
    QString m_text;
    QIcon m_iconChecked;
    QIcon m_iconUnchecked;
    bool m_showIcons = false;
};
