#pragma once

#include <QDialog>
#include <QString>
#include <QList>
#include "qgitbranch.h"
#include "qgittag.h"

namespace Ui {
class QGitExportArchiveDialog;
}

class QGit;

class QGitExportArchiveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitExportArchiveDialog(QGit *git, const QString &selectedRefOrCommit = QString(), QWidget *parent = nullptr);
    ~QGitExportArchiveDialog();

    QString outputFilePath() const;
    QString targetRefOrCommit() const;
    QString prefix() const;
    QString format() const;

    void accept() override;

private slots:
    void on_radioButton_currentHead_toggled(bool checked);
    void on_radioButton_branch_toggled(bool checked);
    void on_radioButton_tag_toggled(bool checked);
    void on_radioButton_commit_toggled(bool checked);
    void on_comboBox_branch_currentIndexChanged(int index);
    void on_comboBox_tag_currentIndexChanged(int index);
    void on_lineEdit_commit_textChanged(const QString &text);
    void on_comboBox_format_currentIndexChanged(int index);
    void on_pushButton_browse_clicked();
    void on_lineEdit_outputFile_textEdited(const QString &text);

private:
    void populateRefs();
    void updateDefaultFileName();
    QString getEffectiveRefName() const;

    Ui::QGitExportArchiveDialog *ui = nullptr;
    QGit *m_git = nullptr;
    QString m_initialRefOrCommit;
    QList<QGitBranch> m_branches;
    QList<QGitTag> m_tags;
    bool m_userModifiedOutputFile = false;
};
