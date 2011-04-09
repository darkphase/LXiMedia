MODULE_NAME = lxistream_dvdnav
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiStream
}

# Files
HEADERS += formatprober.h \
    module.h \
    bufferreader.h
SOURCES += formatprober.cpp \
    module.cpp \
    bufferreader.cpp
