MODULE_NAME = lxistream_mms
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/mms/mms.pri)

QT += network xml

# Files
HEADERS += module.h \
    networkbufferreader.h
SOURCES += module.cpp \
    networkbufferreader.cpp

# Export plugin
SOURCES += export.cpp

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = 41bef008-c663-11e0-be0d-b7661e65f2b3
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
