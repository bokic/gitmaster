#pragma once


#include <QDialog>

namespace Ui {
class QGitPushDialog;
}

class QGitPushDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitPushDialog(QWidget *parent = nullptr);
    ~QGitPushDialog();

private:
    Ui::QGitPushDialog *ui;
};
