#include "qgitmastermainwindow.h"
#include "ui_qgitmastermainwindow.h"

QGitMasterMainWindow::QGitMasterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::QGitMasterMainWindow)
{
    ui->setupUi(this);

    ui->splitter->setSizes(QList<int>() << 1000 << 4000);
}

QGitMasterMainWindow::~QGitMasterMainWindow()
{
    delete ui;
}
