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
