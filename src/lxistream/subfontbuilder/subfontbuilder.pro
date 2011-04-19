TEMPLATE = app

LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = subfontbuilder
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
CONFIG += console

# Files
SOURCES += main.cpp
