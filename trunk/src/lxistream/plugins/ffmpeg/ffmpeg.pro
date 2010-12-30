PLUGIN_NAME = ffmpeg
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)
QT += gui \
    network \
    xml

# Files
HEADERS += audiodecoder.h \
    audioencoder.h \
    audioresampler.h \
    datadecoder.h \
    ffmpegcommon.h \
    formatprober.h \
    module.h \
    videodecoder.h \
    videoencoder.h \
    videoresizer.h \
    bufferwriter.h \
    bufferreader.h
SOURCES += audiodecoder.cpp \
    audiodecoder.postfilter.c \
    audioencoder.cpp \
    audioresampler.cpp \
    datadecoder.cpp \
    ffmpegcommon.cpp \
    formatprober.cpp \
    module.cpp \
    videodecoder.cpp \
    videoencoder.cpp \
    videoresizer.cpp \
    videoencoder.convert.c \
    bufferwriter.cpp \
    bufferreader.cpp
