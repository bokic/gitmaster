#ifndef QGITMASTERMAINWINDOW_H
#define QGITMASTERMAINWINDOW_H

#include <QMainWindow>
#include "qgit.h"

namespace Ui {
class QGitMasterMainWindow;
}

class QGitMasterMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QGitMasterMainWindow(QWidget *parent = 0);
    ~QGitMasterMainWindow();

private:
    void readSettings();

    Ui::QGitMasterMainWindow *ui;
    QGit m_git;
};

#endif // QGITMASTERMAINWINDOW_H
