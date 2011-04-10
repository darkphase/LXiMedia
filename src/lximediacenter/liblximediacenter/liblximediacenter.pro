TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiMediaCenter
include($${LXIMEDIA_DIR}/include/config.pri)
QT += sql

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblximediacenter
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblximediacenter

DEFINES += S_BUILD_LIBLXIMEDIACENTER

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiMediaCenter
}

include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

# Files
HEADERS += ../../../include/LXiMediaCenter \
 ../../../include/liblximediacenter/export.h \
 ../../../include/liblximediacenter/backendsandbox.h \
 ../../../include/liblximediacenter/backendserver.h \
 ../../../include/liblximediacenter/database.h \
 ../../../include/liblximediacenter/globalsettings.h \
 ../../../include/liblximediacenter/htmlparser.h \
 ../../../include/liblximediacenter/imdbclient.h \
 ../../../include/liblximediacenter/mediaserver.h \
 ../../../include/liblximediacenter/mediastream.h \
 ../../../include/liblximediacenter/pluginsettings.h \
 ../../../include/liblximediacenter/teletext.h

SOURCES += backendsandbox.cpp \
 backendserver.cpp \
 backendserver.html.cpp \
 database.cpp \
 globalsettings.cpp \
 htmlparser.cpp \
 imdbclient.cpp \
 mediaserver.cpp \
 mediaserver.html.cpp \
 mediastream.cpp \
 pluginsettings.cpp \
 teletext.cpp

HEADERS += lximediacenterprivate.h
SOURCES += lximediacenterprivate.cpp

RESOURCES = images/lximediacenter_images.qrc

unix {
    LIBS += -lz
    target.path = /usr/lib
    INSTALLS += target
}

win32 {
  LIBS += -lws2_32
}

win32-g++ {
  system(mkdir ..\\..\\..\\bin\\ > NUL 2>&1)
  system(mkdir ..\\..\\..\\bin\\sqldrivers\\ > NUL 2>&1)

  release {
    system(cp -u $$(QTDIR)/bin/QtSql4.dll -t $${LXIMEDIA_DIR}/bin)
    system(cp -u $$(QTDIR)/plugins/sqldrivers/qsqlite4.dll -t $${LXIMEDIA_DIR}/bin/sqldrivers)
  }
}
