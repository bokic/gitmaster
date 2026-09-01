#include "qgitmastermainwindow.h"

#include <QProcessEnvironment>
//#include <QStyleFactory>
#include <QApplication>

#include <cstdlib>

#include <git2.h>


int main(int argc, char *argv[])
{
    int ret = EXIT_SUCCESS;

    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");

    git_libgit2_init();

    QApplication a(argc, argv);

/*    auto sty = QStyleFactory::keys();
#ifdef Q_OS_UNIX
    if (QProcessEnvironment::systemEnvironment().value("XDG_CURRENT_DESKTOP") == "XFCE")
        QApplication::setStyle("gtk2");
    else
        QApplication::setStyle("Fusion");
#endif*/

    QGitMasterMainWindow w;
    w.show();

    ret = QApplication::exec();

    git_libgit2_shutdown();

    return ret;
}
