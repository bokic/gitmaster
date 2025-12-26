#pragma once

#include <QComboBox>
#include <QWidget>
#include <QString>
#include <QIcon>


class QComboBoxGitStatusFiles : public QComboBox
{
    Q_OBJECT
public:
    enum QComboBoxGitStatusFilesShowFiles {QShowPendingFiles, QShowConflictFiles, QShowUntracked, QShowIgnored, QShowClean, QShowModified, QShowAll};
    enum QComboBoxGitStatusFilesOrderFiles {QUnsortedFiles, QFilePathSortFiles, QReversedFilePathSortFiles, QFileNameSortFiles, QReversedFileNameSortFiles, QFileStatusSortFiles, QCheckedUncheckedSortFiles};

    explicit QComboBoxGitStatusFiles(QWidget *parent = nullptr);
    ~QComboBoxGitStatusFiles() = default;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void updateText();
    QComboBoxGitStatusFilesShowFiles showFiles() const;
    QComboBoxGitStatusFilesOrderFiles showSortBy() const;

signals:
    void itemClicked(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void showPopup() override;

private Q_SLOTS:
    void activated(int index);

private:
    QString m_text;
    QIcon m_iconChecked;
    QIcon m_iconUnchecked;
    bool m_showIcons = false;
};
