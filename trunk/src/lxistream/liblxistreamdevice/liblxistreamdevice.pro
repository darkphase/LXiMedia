TEMPLATE = lib
CONFIG += dll
QT -= gui
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiStreamDevice
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice

DEFINES += S_BUILD_LIBLXISTREAMDEVICE

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStreamDevice

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiStreamDevice \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/export.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/sinterfaces.h
SOURCES += sinterfaces.cpp

# Nodes
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/nodes/saudioinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/nodes/saudiooutputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/nodes/saudiovideoinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/nodes/svideoinputnode.h
SOURCES += nodes/saudioinputnode.cpp \
    nodes/saudiooutputnode.cpp \
    nodes/saudiovideoinputnode.cpp \
    nodes/svideoinputnode.cpp

HEADERS += lxistreamdeviceprivate.h
SOURCES += lxistreamdeviceprivate.cpp

# Platform specific
unix { 
    target.path = /usr/lib
    INSTALLS += target
}

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = a412c2e6-892f-11e0-8c25-132c45a4d138
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
