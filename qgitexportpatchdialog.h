#pragma once

#include <QDialog>
#include <QStringList>
#include "qgitcommit.h"

namespace Ui {
class QGitExportPatchDialog;
}

class QGit;

class QGitExportPatchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitExportPatchDialog(QGit *git, const QString &selectedCommitHash = QString(), QWidget *parent = nullptr);
    ~QGitExportPatchDialog();

    QString outputDirectory() const;
    QStringList selectedCommitIds() const;
    QString subjectPrefix() const;
    bool isNumbered() const;
    bool detectRenames() const;

    void accept() override;

private slots:
    void on_radioButton_selected_toggled(bool checked);
    void on_radioButton_rangeToHead_toggled(bool checked);
    void on_radioButton_lastN_toggled(bool checked);
    void on_radioButton_custom_toggled(bool checked);
    void on_spinBox_lastN_valueChanged(int value);
    void on_lineEdit_custom_textChanged(const QString &text);
    void on_pushButton_browse_clicked();

private:
    void updateCommitList();
    void populateTable(const QList<QGitCommit> &commits);

    Ui::QGitExportPatchDialog *ui = nullptr;
    QGit *m_git = nullptr;
    QString m_selectedHash;
    QList<QGitCommit> m_currentCommits;
};
