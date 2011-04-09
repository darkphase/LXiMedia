MODULE_NAME = lxistream_v4l
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiStream
}

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
#SOURCES += $${LXIMEDIA_DIR}/ext/alevt-1.6.1/cache.c \
# $${LXIMEDIA_DIR}/ext/alevt-1.6.1/fdset.c \
# $${LXIMEDIA_DIR}/ext/alevt-1.6.1/hamm.c \
# $${LXIMEDIA_DIR}/ext/alevt-1.6.1/lang.c \
# $${LXIMEDIA_DIR}/ext/alevt-1.6.1/misc.c \
# $${LXIMEDIA_DIR}/ext/alevt-1.6.1/vbi.c
