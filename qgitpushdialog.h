#pragma once


#include "qgitrepository.h"

#include <QDialog>


namespace Ui {
class QGitPushDialog;
}

class QGitPushDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitPushDialog(QGitRepository *parent = nullptr);
    ~QGitPushDialog();

private:
    Ui::QGitPushDialog *ui = nullptr;
};
