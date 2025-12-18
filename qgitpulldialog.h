#pragma once

#include <QDialog>

namespace Ui {
class QGitPullDialog;
}

class QGitPullDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitPullDialog(QWidget *parent = nullptr);
    ~QGitPullDialog();

private:
    Ui::QGitPullDialog *ui = nullptr;
};
