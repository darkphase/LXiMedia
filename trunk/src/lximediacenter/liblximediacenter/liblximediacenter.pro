TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiMediaCenter
include($${LXIMEDIA_DIR}/include/config.pri)
QT += sql

# Generate version.h
unix {
  system(mkdir -p $${OBJECTS_DIR})
  system(echo \\\"$${VERSION}\\\" > $${OBJECTS_DIR}/version.h)
  INCLUDEPATH += $${OBJECTS_DIR}/
}

win32 {
  BSOBJECTS_DIR = $$replace(OBJECTS_DIR, /, \\)
  system(if not exist $${BSOBJECTS_DIR} mkdir $${BSOBJECTS_DIR})
  system(echo \"$${VERSION}\" > $${BSOBJECTS_DIR}\\version.h)
  INCLUDEPATH += $${OBJECTS_DIR}/
}

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblximediacenter
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblximediacenter

linux-g++|win32-g++ {
  # Generate/Use precompiled header
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiMediaCenter
}

include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)

# Files
HEADERS += ../../../include/LXiMediaCenter \
 ../../../include/liblximediacenter/backendserver.h \
 ../../../include/liblximediacenter/database.h \
 ../../../include/liblximediacenter/dlnaserver.h \
 ../../../include/liblximediacenter/globalsettings.h \
 ../../../include/liblximediacenter/htmlparser.h \
 ../../../include/liblximediacenter/httpoutputnode.h \
 ../../../include/liblximediacenter/httpserver.h \
 ../../../include/liblximediacenter/imdbclient.h \
 ../../../include/liblximediacenter/mediaserver.h \
 ../../../include/liblximediacenter/plugininterfaces.h \
 ../../../include/liblximediacenter/pluginsettings.h \
 ../../../include/liblximediacenter/ssdpclient.h \
 ../../../include/liblximediacenter/ssdpserver.h \
 ../../../include/liblximediacenter/teletext.h

SOURCES += backendserver.cpp \
 backendserver.html.cpp \
 database.cpp \
 dlnaserver.cpp \
 globalsettings.cpp \
 htmlparser.cpp \
 httpoutputnode.cpp \
 httpserver.cpp \
 httpserver.header.cpp \
 imdbclient.cpp \
 mediaserver.cpp \
 mediaserver.html.cpp \
 plugininterfaces.cpp \
 pluginsettings.cpp \
 ssdpclient.cpp \
 ssdpserver.cpp \
 teletext.cpp

RESOURCES = images/lximediacenter_images.qrc \
 dlnaserver.qrc

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
