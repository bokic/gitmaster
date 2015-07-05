#ifndef QNEWREPOSITORYDIALOG_H
#define QNEWREPOSITORYDIALOG_H

#include <QDialog>

namespace Ui {
class QNewRepositoryDialog;
}

class QNewRepositoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QNewRepositoryDialog(QWidget *parent = 0);
    ~QNewRepositoryDialog();

private slots:
    void on_toolButtonCloneRepositoryBrowse_clicked();
    void on_toolButtonAddWorkingCopyBrowse_clicked();
    void on_toolButtonCreateNewRepositoryBrowse_clicked();

private:
    Ui::QNewRepositoryDialog *ui;
};

#endif // QNEWREPOSITORYDIALOG_H
