#ifndef QGITBRANCHDIALOG_H
#define QGITBRANCHDIALOG_H

#include <QDialog>

namespace Ui {
class QGitBranchDialog;
}

class QGitBranchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitBranchDialog(QWidget *parent = nullptr);
    ~QGitBranchDialog();

private:
    Ui::QGitBranchDialog *ui;
};

#endif // QGITBRANCHDIALOG_H
