PLUGIN_NAME = ffmpeg
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxistream/plugin.pri)
unix:include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream-internal.pri)
win32:include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri) # Because of -mstackrealign
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)

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
    bufferreader.h \
    videoformatconverter.h
SOURCES += audiodecoder.cpp \
    audioencoder.cpp \
    audioresampler.cpp \
    datadecoder.cpp \
    ffmpegcommon.cpp \
    formatprober.cpp \
    module.cpp \
    videodecoder.cpp \
    videoencoder.cpp \
    videoresizer.cpp \
    bufferwriter.cpp \
    bufferreader.cpp \
    videoformatconverter.cpp
