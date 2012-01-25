TEMPLATE = app
win32:CONFIG += console
QT -= gui
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = licensetool
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

# Files
HEADERS += license.h
SOURCES += main.cpp

# Run after link
unix:QMAKE_LFLAGS += -Wl,-rpath -Wl,.
unix:QMAKE_POST_LINK = cd $${DESTDIR} && ./$${TARGET} $${PWD}/license.h \
  $${PWD}/$${LXIMEDIA_DIR}/src \
  $${PWD}/$${LXIMEDIA_DIR}/include \
  $${PWD}/$${LXIMEDIA_DIR}/test
