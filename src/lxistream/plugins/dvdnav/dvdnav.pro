MODULE_NAME = lxistream_dvdnav
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)

# Files
HEADERS += formatprober.h \
    module.h \
    bufferreader.h
SOURCES += formatprober.cpp \
    module.cpp \
    bufferreader.cpp

# Export plugin
SOURCES +=

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = f9ffb2be-7358-11e0-a6bd-fb96216d1d8a
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
