#include "qgitmastermainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGitMasterMainWindow w;
    w.show();

    return a.exec();
}
