#include "qgitmastermainwindow.h"
#include <QApplication>
#include <QList>
#include <QMap>
#include <QDir>

void setAppSettings()
{
    QCoreApplication::setOrganizationName("BorisBarbulovski");
    QCoreApplication::setApplicationName("GitMaster");
}

void registerTypes()
{
    qRegisterMetaType<QDir>("QDir");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<QMap<git_status_t,int>>("QMap<git_status_t,int>");
    qRegisterMetaType<QMap<QString,git_status_t>>("QMap<QString,git_status_t>");
    qRegisterMetaType<QList<QGitBranch>>("QList<QGitBranch>");
    qRegisterMetaType<QList<QGitCommit>>("QList<QGitCommit>");
    qRegisterMetaType<QList<QGitCommit>>("QGitError");
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
