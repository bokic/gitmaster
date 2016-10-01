lessThan(QT_MAJOR_VERSION, 5) {
    error("Unsupported Qt version(Needs Qt5).")
}

TEMPLATE  = lib
QT        = core
CONFIG   += debug_and_release c++11
TARGET    = qgitlib
DESTDIR   = $$_PRO_FILE_PWD_/../bin
VERSION   = 0.1.0.0

DEPENDPATH  += ../include
INCLUDEPATH += ../include

SOURCES     +=            qgit.cpp
HEADERS     += ../include/qgit.h

SOURCES     +=            qgitbranch.cpp
HEADERS     += ../include/qgitbranch.h

SOURCES     +=            qgitcommit.cpp
HEADERS     += ../include/qgitcommit.h

SOURCES     +=            qgitcommitdiff.cpp
HEADERS     += ../include/qgitcommitdiff.h

SOURCES     +=            qgitdifffile.cpp
HEADERS     += ../include/qgitdifffile.h

SOURCES     +=            qgitdiffhunk.cpp
HEADERS     += ../include/qgitdiffhunk.h

SOURCES     +=            qgitdifffileitem.cpp
HEADERS     += ../include/qgitdifffileitem.h

SOURCES     +=            qgitdiffline.cpp
HEADERS     += ../include/qgitdiffline.h

SOURCES     +=            qgitsignature.cpp
HEADERS     += ../include/qgitsignature.h

SOURCES     +=            qgiterror.cpp
HEADERS     += ../include/qgiterror.h

LIBS        += -lgit2
