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
    void refreshAll();
    bool hasBranch(const QString &name) const;
    QString detectMainBranch() const;
    bool ensureDevelopBranch();
    void updateStartButtonState(class QLineEdit *edit, class QPushButton *button, const QString &prefix);
    void startFlowBranch(const QString &prefix, class QLineEdit *nameEdit, bool useDevelopBase, const QString &kindLabel);
    void finishReleaseOrHotfix(const QString &branchName, const QString &prefix, const QString &kindLabel);

    Ui::QGitFlowDialog *ui = nullptr;
    QGitRepository *m_repo = nullptr;
    QGit m_git;
};
