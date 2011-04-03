MODULE_NAME = lxistream.dvdnav
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream-internal.pri)
include($${LXIMEDIA_DIR}/ext/dvdnav/dvdnav.pri)

# Files
HEADERS += formatprober.h \
    module.h \
    bufferreader.h
SOURCES += formatprober.cpp \
    module.cpp \
    bufferreader.cpp
