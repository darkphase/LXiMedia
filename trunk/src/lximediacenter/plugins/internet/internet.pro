MODULE_NAME = lximediacenter_internet
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)
QT += network xml sql script

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter

# Files
HEADERS += configserver.h \
    internetserver.h \
    module.h \
    scriptengine.h \
    sitedatabase.h

SOURCES += configserver.cpp \
    internetserver.cpp \
    configserver.html.cpp \
    module.cpp \
    scriptengine.cpp \
    sitedatabase.cpp

RESOURCES = internet.qrc

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = c1ad044e-89be-11e0-b796-070444e1d3e8
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
