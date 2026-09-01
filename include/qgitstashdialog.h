#pragma once

#include <QDialog>


namespace Ui {
class QGitStashDialog;
}

class QGitStashDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QGitStashDialog(QWidget *parent = nullptr);
    ~QGitStashDialog();

    QString message() const;
    bool keepIndex() const;
    bool includeUntracked() const;
    bool includeIgnored() const;

private:
    Ui::QGitStashDialog *ui = nullptr;
};
