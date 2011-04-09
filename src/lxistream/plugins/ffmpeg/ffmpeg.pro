MODULE_NAME = lxistream_ffmpeg
LXIMEDIA_DIR = ../../../..
include($${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiStream
}

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
