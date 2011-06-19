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

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter

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

RESOURCES = liblximediacenter.qrc

unix {
  LIBS += -lz

  !macx {
    target.path = /usr/lib
    INSTALLS += target
  }
}

win32 {
  LIBS += -lws2_32
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  system(mkdir $${OUT_DIR}\\sqldrivers > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\QtCore4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtSql4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtXml4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\sqldrivers\\qsqlite4.dll $${OUT_DIR}\\sqldrivers > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtCored4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtSqld4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\QtXmld4.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\plugins\\sqldrivers\\qsqlited4.dll $${OUT_DIR}\\sqldrivers > NUL)
  }
}
win32-g++ {
    system(copy /Y $$(QTDIR)\\bin\\libgcc_s_dw2-1.dll $${OUT_DIR} > NUL)
    system(copy /Y $$(QTDIR)\\bin\\mingwm10.dll $${OUT_DIR} > NUL)
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = f4e525ea-7365-11e0-aef3-cba632b16f97
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
