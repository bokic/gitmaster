#pragma once

#include <QDialog>
#include <QWidget>


namespace Ui {
class QGitFetchDialog;
}

class QGitFetchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitFetchDialog(QWidget *parent = nullptr);
    ~QGitFetchDialog();

private:
    Ui::QGitFetchDialog *ui = nullptr;
};
