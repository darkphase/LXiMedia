MODULE_NAME = lxistream.fftw
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream-internal.pri)
include($${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

# Files
HEADERS += module.h

SOURCES += module.cpp
