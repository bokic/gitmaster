#include "qgitmastermainwindow.h"
#include <QApplication>

void setAppSettings()
{
    QCoreApplication::setOrganizationName("BorisBarbulovski");
    QCoreApplication::setApplicationName("GitMaster");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setAppSettings();

    QGitMasterMainWindow w;
    w.show();

    return a.exec();
}
