#include "qgitmastermainwindow.h"
#include "qgitbranch.h"
#include "qgitcommit.h"
#include "qgiterror.h"
#include "qgit.h"

#include <QProcessEnvironment>
#include <QApplication>
#include <QVector>
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
    qRegisterMetaType<QVector<QGitDiffWidgetLine>>("QVector<QGitDiffWidgetLine>");
    qRegisterMetaType<QList<QGitDiffFile>>("QList<QGitDiffFile>");
    qRegisterMetaType<QList<QGitBranch>>("QList<QGitBranch>");
    qRegisterMetaType<QList<QGitCommit>>("QList<QGitCommit>");
    qRegisterMetaType<QList<QString>>("QList<QString>");
    qRegisterMetaType<QGitCommit>("QGitCommit");
    qRegisterMetaType<QGitError>("QGitError");
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef Q_OS_UNIX
    if (QProcessEnvironment::systemEnvironment().value("XDG_CURRENT_DESKTOP") == "XFCE")
        QApplication::setStyle("gtk2");
#endif

    setAppSettings();
    registerTypes();

    QGitMasterMainWindow w;
    w.show();

    return QApplication::exec();
}
