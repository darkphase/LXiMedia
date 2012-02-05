TEMPLATE = app
CONFIG += qtestlib
win32:CONFIG += console
QT -= gui
QT += network
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lxiservertest
INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)

# Files
HEADERS += httpenginetest.h \
    sandboxtest.h
SOURCES += main.cpp \
    httpenginetest.cpp \
    sandboxtest.cpp

# Run tests after link
unix:QMAKE_LFLAGS += -Wl,-rpath -Wl,.
unix:QMAKE_POST_LINK = cd $${DESTDIR} && ./$${TARGET} -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\QtTest4.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtTestd4.dll $${OUT_DIR} > NUL)
  }
}

win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vcapp
  GUID = cbbc176e-3a3e-11e1-9f6e-332020b652d8
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
