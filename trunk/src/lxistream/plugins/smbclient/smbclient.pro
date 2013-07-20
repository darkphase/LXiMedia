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
SOURCES +=

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = abd0f006-3a38-11e1-9457-6f298440b8ae
  DEFINES += _CRT_SECURE_NO_WARNINGS
  PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
}
