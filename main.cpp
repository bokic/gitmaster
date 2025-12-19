#include "qgitmastermainwindow.h"

#include <QProcessEnvironment>
#include <QApplication>


int main(int argc, char *argv[])
{
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");
    QApplication a(argc, argv);

/*#ifdef Q_OS_UNIX
    if (QProcessEnvironment::systemEnvironment().value("XDG_CURRENT_DESKTOP") == "XFCE")
        QApplication::setStyle("gtk2");
    else
        QApplication::setStyle("Adwaita");
#endif*/

    QGitMasterMainWindow w;
    w.show();

    return QApplication::exec();
}
