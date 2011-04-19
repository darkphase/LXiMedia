TEMPLATE = lib
CONFIG += dll
QT += network xml sql
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiMediaCenter
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter

DEFINES += S_BUILD_LIBLXIMEDIACENTER

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter
}

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/export.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/backendsandbox.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/backendserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/database.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/globalsettings.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/htmlparser.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/imdbclient.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/mediaserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/mediastream.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/pluginsettings.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/teletext.h

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
  system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
  system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\sqldrivers\\ > NUL 2>&1)

  release {
    system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    system(cp -u $$(QTDIR)/bin/QtSql4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    system(cp -u $$(QTDIR)/plugins/sqldrivers/qsqlite4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/sqldrivers)
  }
  debug {
    system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    system(cp -u $$(QTDIR)/bin/QtSqld4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    system(cp -u $$(QTDIR)/plugins/sqldrivers/qsqlited4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/sqldrivers)
  }
}
