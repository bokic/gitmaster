#ifndef QGITMASTERMAINWINDOW_H
#define QGITMASTERMAINWINDOW_H

#include <QMainWindow>

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
    Ui::QGitMasterMainWindow *ui;
};

#endif // QGITMASTERMAINWINDOW_H
