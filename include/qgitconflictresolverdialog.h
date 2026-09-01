#pragma once

#include <QDialog>
#include <QString>
#include <qgit.h>

namespace Ui {
class QGitConflictResolverDialog;
}

class QGitConflictResolverDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitConflictResolverDialog(QGit *git, const QString &filePath, QWidget *parent = nullptr);
    ~QGitConflictResolverDialog() override;

private slots:
    void on_listConflicts_currentRowChanged(int currentRow);
    void on_pushButton_useOurs_clicked();
    void on_pushButton_useTheirs_clicked();
    void on_pushButton_resolve_clicked();
    void on_pushButton_cancel_clicked();
    void syncScrollLeft(int value);
    void syncScrollMiddle(int value);
    void syncScrollRight(int value);

private:
    void loadConflictFiles();
    void refreshConflictList();
    void scrollToConflict(int index);
    void resolveConflictBlock(int index, bool useOurs);

    Ui::QGitConflictResolverDialog *ui;
    QGit *m_git;
    QString m_filePath;
};
