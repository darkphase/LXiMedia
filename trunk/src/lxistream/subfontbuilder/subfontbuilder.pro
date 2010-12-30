TEMPLATE = app

LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = subfontbuilder
include($${LXIMEDIA_DIR}/include/config.pri)
CONFIG += console

# Files
SOURCES += main.cpp
