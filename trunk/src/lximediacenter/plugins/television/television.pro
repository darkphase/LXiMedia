PLUGIN_NAME = television
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblximediacenter/plugin.pri)

# Files
SOURCES += cameraserver.cpp \
 cameraserver.html.cpp \
# configserver.cpp \
# configserver.html.cpp \
# epgdatabase.cpp \
# scan.cpp \
# teletextserver.cpp \
# teletextserver.html.cpp \
 televisionbackend.cpp \
# televisionserver.cpp \
# televisionserver.html.cpp

HEADERS += cameraserver.h \
# configserver.h \
# epgdatabase.h \
# scan.h \
# teletextserver.h \
 televisionbackend.h \
# televisionserver.h
