MODULE_NAME = lxistream_fftw
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

# Files
HEADERS += module.h

SOURCES += module.cpp

# Export plugin
SOURCES += export.cpp

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = 2a5cfcb4-7359-11e0-bdef-b7a999f73b97
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
