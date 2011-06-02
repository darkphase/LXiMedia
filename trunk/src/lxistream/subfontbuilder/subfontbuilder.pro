TEMPLATE = app

LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = subfontbuilder
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
CONFIG += console
macx:CONFIG -= app_bundle

# Files
SOURCES += main.cpp

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vcapp
  GUID = 25fb8d8c-734c-11e0-80ed-037669f892f1
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
