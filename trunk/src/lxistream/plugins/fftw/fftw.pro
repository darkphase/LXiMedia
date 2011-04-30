MODULE_NAME = lxistream_fftw
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

# Files
HEADERS += module.h

SOURCES += module.cpp
