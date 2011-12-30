MODULE_NAME = lxistream_smbclient
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/smbclient/smbclient.pri)

# Files
HEADERS += module.h \
    smbfilesystem.h
SOURCES += module.cpp \
    smbfilesystem.cpp

# Export plugin
SOURCES += export.cpp
