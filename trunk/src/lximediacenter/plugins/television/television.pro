MODULE_NAME = lximediacenter_television
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)
QT += network xml sql

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter

# Files
SOURCES += cameraserver.cpp \
# configserver.cpp \
# configserver.html.cpp \
# epgdatabase.cpp \
# scan.cpp \
# teletextserver.cpp \
# teletextserver.html.cpp \
 module.cpp \
 televisionsandbox.cpp \
# televisionserver.cpp \
# televisionserver.html.cpp

HEADERS += cameraserver.h \
# configserver.h \
# epgdatabase.h \
# scan.h \
# teletextserver.h \
 module.h \
 televisionsandbox.h \
# televisionserver.h

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = c7680d90-8935-11e0-8111-bbf01c3789c5
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
