#ifndef QGITMASTERMAINWINDOW_H
#define QGITMASTERMAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
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

private slots:
    void on_actionCloneNew_triggered();
    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    void readSettings();

    Ui::QGitMasterMainWindow *ui;
    QGit m_git;
};

#endif // QGITMASTERMAINWINDOW_H
