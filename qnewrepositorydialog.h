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

private:
    Ui::QNewRepositoryDialog *ui;
};

#endif // QNEWREPOSITORYDIALOG_H
