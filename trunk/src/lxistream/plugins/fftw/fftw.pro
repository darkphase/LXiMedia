PLUGIN_NAME = fftw
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${LXIMEDIA_DIR}/ext/fftw/fftw.pri)
QT += gui xml

# Files
HEADERS += module.h

SOURCES += module.cpp