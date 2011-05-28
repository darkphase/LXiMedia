MODULE_NAME = lxistreamdevice_v4l
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamdevice/linklxistreamdevice.pri)

# Files
HEADERS += videodev.h \
 videodev2.h

HEADERS += module.h \
# v4l2device.h \
 v4l2input.h \
# v4l2tuner.h \
# v4lcommon.h \
# vbiinput.h

SOURCES += module.cpp \
# v4l2device.cpp \
 v4l2input.cpp \
# v4l2tuner.cpp \
# v4lcommon.cpp \
# vbiinput.cpp

# AleVt
#SOURCES += $${PWD}/$${LXIMEDIA_DIR}/ext/alevt-1.6.1/cache.c \
# $${PWD}/$${LXIMEDIA_DIR}/ext/alevt-1.6.1/fdset.c \
# $${PWD}/$${LXIMEDIA_DIR}/ext/alevt-1.6.1/hamm.c \
# $${PWD}/$${LXIMEDIA_DIR}/ext/alevt-1.6.1/lang.c \
# $${PWD}/$${LXIMEDIA_DIR}/ext/alevt-1.6.1/misc.c \
# $${PWD}/$${LXIMEDIA_DIR}/ext/alevt-1.6.1/vbi.c
