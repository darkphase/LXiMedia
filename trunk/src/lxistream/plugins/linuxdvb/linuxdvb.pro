# Globals
PLUGIN_NAME = linuxdvb
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

# Files
HEADERS += dvbdevice.h \
 dvbinput.h \
 dvbtuner.h \
 module.h

SOURCES += dvbdevice.cpp \
 dvbinput.cpp \
 dvbtuner.cpp \
 module.cpp

# Export plugin
SOURCES += export.cpp

# AleVt
SOURCES += $${LXIMEDIA_DIR}/ext/alevt-1.6.1/hamm.c \
 $${LXIMEDIA_DIR}/ext/alevt-1.6.1/lang.c
