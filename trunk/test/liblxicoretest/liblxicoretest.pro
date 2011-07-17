TEMPLATE = app
CONFIG += qtestlib
win32:CONFIG += console
QT -= gui
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
    system(copy /Y $$(QTDIR)\\bin\\QtTest4.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtTestd4.dll $${OUT_DIR} > NUL)
  }
}
