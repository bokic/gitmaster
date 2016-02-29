#pragma once

#include <QDialog>

namespace Ui {
class QNewRepositoryDialog;
}

class QNewRepositoryDialog : public QDialog
{
    Q_OBJECT

public:
    enum {QCloneRepository, QAddWorkingCopy, QCreateNewRepository};
    explicit QNewRepositoryDialog(QWidget *parent = 0);
    ~QNewRepositoryDialog();
    int tabCurrentIndex() const;
    QString cloneRepositorySourceURL() const;
    QString cloneRepositoryDestinationPath() const;
    QString cloneRepositoryBookmarkText() const;
    QString addWorkingCopyPath() const;
    QString addWorkingCopyBookmarkText() const;
    QString createNewRepositoryPath() const;
    QString createNewRepositoryBookmarkText() const;

private slots:
    void updateWidgets();
    void on_toolButtonCloneRepositoryBrowse_clicked();
    void on_toolButtonAddWorkingCopyBrowse_clicked();
    void on_toolButtonCreateNewRepositoryBrowse_clicked();
    void on_checkBoxCloneRepositoryBookmark_stateChanged(int state);
    void on_checkBoxAddWorkingCopyBookmark_stateChanged(int state);
    void on_checkBoxCreateNewRepositoryBookmark_stateChanged(int state);

private:
    Ui::QNewRepositoryDialog *ui;
};
