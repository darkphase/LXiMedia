TEMPLATE = app
QT += testlib
QT -= gui
win32:CONFIG += console
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lxicoretest
INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)

# Files
HEADERS += coretest.h
SOURCES += main.cpp \
    coretest.cpp

# Run tests after link
unix:QMAKE_LFLAGS += -Wl,-rpath -Wl,.
unix:QMAKE_POST_LINK = cd $${DESTDIR} && ./$${TARGET} -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Test.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Testd.dll $${OUT_DIR} > NUL)
  }

  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vcapp
    GUID = 56dc9e00-3a3e-11e1-9fa9-2b90dc4a8729
    DEFINES += _CRT_SECURE_NO_WARNINGS
  }
}
