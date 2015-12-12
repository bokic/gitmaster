#include "qgitmastermainwindow.h"
#include "structs.h"
#include <QApplication>
#include <QHash>
#include <QDir>

void setAppSettings()
{
    QCoreApplication::setOrganizationName("BorisBarbulovski");
    QCoreApplication::setApplicationName("GitMaster");
}

void registerTypes()
{
    qRegisterMetaType<QDir>("QDir");
    qRegisterMetaType<QHash<git_status_t,int>>("QHash<git_status_t,int>");
    qRegisterMetaType<QList<QGitBranch>>("QList<QGitBranch>");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setAppSettings();
    registerTypes();

    QGitMasterMainWindow w;
    w.show();

    return a.exec();
}
