#pragma once

#include <QDialog>
#include "qgit.h"

namespace Ui {
class QGitFlowDialog;
}

class QGitRepository;

class QGitFlowDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitFlowDialog(QGitRepository *parent = nullptr);
    ~QGitFlowDialog();

private slots:
    void on_pushButton_startFeature_clicked();
    void on_pushButton_finishFeature_clicked();
    void on_pushButton_startRelease_clicked();
    void on_pushButton_finishRelease_clicked();
    void on_pushButton_startHotfix_clicked();
    void on_pushButton_finishHotfix_clicked();
    void on_lineEdit_featureName_textChanged(const QString &text);
    void on_lineEdit_releaseName_textChanged(const QString &text);
    void on_lineEdit_hotfixName_textChanged(const QString &text);

private:
    void refreshBranches();
    bool hasBranch(const QString &name) const;
    QString detectMainBranch() const;
    void ensureDevelopBranch();

    Ui::QGitFlowDialog *ui;
    QGitRepository *m_repo;
    QGit m_git;
};
