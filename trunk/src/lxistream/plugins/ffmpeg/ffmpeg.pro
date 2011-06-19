MODULE_NAME = lxistream_ffmpeg
LXIMEDIA_DIR = ../../../..
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/module.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)

CONFIG += precompile_header
PRECOMPILED_HEADER = ffmpegcommon.h

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

# Export plugin
SOURCES += export.cpp

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = 1cad8fa2-7359-11e0-9142-974909def34c
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
