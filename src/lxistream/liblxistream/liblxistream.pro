TEMPLATE = lib
CONFIG += dll
QT += concurrent
QT -= gui
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiStream
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream

DEFINES += S_BUILD_LIBLXISTREAM

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStream

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiStream \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/export.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/saudiobuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/saudiocodec.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/saudioformat.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sbuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sdatabuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sdatacodec.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sencodedaudiobuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sencodeddatabuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sencodedvideobuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sgraph.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sinterfaces.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sinterval.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/smediafilesystem.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/smediainfo.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/ssize.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/ssubpicturebuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/ssubtitlebuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/stime.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/stimer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/svideobuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/svideocodec.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/svideoformat.h
SOURCES += saudiobuffer.cpp \
    saudiocodec.cpp \
    saudioformat.cpp \
    sbuffer.cpp \
    sdatabuffer.cpp \
    sdatacodec.cpp \
    sencodedaudiobuffer.cpp \
    sencodeddatabuffer.cpp \
    sencodedvideobuffer.cpp \
    sgraph.cpp \
    sinterfaces.cpp \
    sinterval.cpp \
    smediafilesystem.cpp \
    smediainfo.cpp \
    ssize.cpp \
    ssubpicturebuffer.cpp \
    ssubtitlebuffer.cpp \
    stime.cpp \
    stimer.cpp \
    svideobuffer.cpp \
    svideocodec.cpp \
    svideoformat.cpp

# Nodes
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiodecodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioencodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioformatconvertnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiogapremovernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiomatrixnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudionormalizenode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioresamplenode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sbufferdeserializernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sbufferserializernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sdatadecodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileoutputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sioinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/siooutputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/snetworkinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/splaylistnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/ssubpicturerendernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/ssubtitleinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/ssubtitlerendernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/stimestampresamplernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/stimestampsyncnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoboxnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideodecodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideodeinterlacenode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoencodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoformatconvertnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoletterboxdetectnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoresizenode.h
SOURCES += nodes/saudiodecodernode.cpp \
    nodes/saudioencodernode.cpp \
    nodes/saudioformatconvertnode.cpp \
    nodes/saudiogapremovernode.cpp \
    nodes/saudiomatrixnode.cpp \
    nodes/saudionormalizenode.cpp \
    nodes/saudioresamplenode.cpp \
    nodes/sbufferdeserializernode.cpp \
    nodes/sbufferserializernode.cpp \
    nodes/sdatadecodernode.cpp \
    nodes/sfileinputnode.cpp \
    nodes/sfileoutputnode.cpp \
    nodes/sinputnode.cpp \
    nodes/sioinputnode.cpp \
    nodes/siooutputnode.cpp \
    nodes/snetworkinputnode.cpp \
    nodes/splaylistnode.cpp \
    nodes/ssubpicturerendernode.cpp \
    nodes/ssubtitleinputnode.cpp \
    nodes/ssubtitlerendernode.cpp \
    nodes/stimestampresamplernode.cpp \
    nodes/stimestampsyncnode.cpp \
    nodes/svideoboxnode.cpp \
    nodes/svideodecodernode.cpp \
    nodes/svideodeinterlacenode.cpp \
    nodes/svideoencodernode.cpp \
    nodes/svideoformatconvertnode.cpp \
    nodes/svideoletterboxdetectnode.cpp \
    nodes/svideoresizenode.cpp

HEADERS += lxistreamprivate.h
SOURCES += lxistreamprivate.cpp

# Common backend classes
HEADERS += common/audioformatconverter.h \
    common/audiooutput.h \
    common/audioresampler.h \
    common/deinterlace.h \
    common/formatprober.h \
    common/localfilesystem.h \
    common/mpeg.h \
    common/pcmaudiodecoder.h \
    common/pcmaudioencoder.h \
    common/psbufferreader.h \
    common/psbufferwriter.h \
    common/rawsubtitledecoder.h \
    common/subtitlereader.h \
    common/tsbufferwriter.h \
    common/videoformatconverter.h
SOURCES += common/audioformatconverter.cpp \
    common/audiooutput.cpp \
    common/audioresampler.cpp \
    common/deinterlace.cpp \
    common/formatprober.cpp \
    common/localfilesystem.cpp \
    common/mpeg.cpp \
    common/pcmaudiodecoder.cpp \
    common/pcmaudioencoder.cpp \
    common/psbufferreader.cpp \
    common/psbufferwriter.cpp \
    common/rawsubtitledecoder.cpp \
    common/subtitlereader.cpp \
    common/tsbufferwriter.cpp \
    common/videoformatconverter.cpp

HEADERS += common/module.h
SOURCES += common/module.cpp

include($${PWD}/$${LXIMEDIA_DIR}/src/lxistream/algorithms/linkalgorithms.pri)

# Platform specific
unix {
  !macx {
    target.path = /usr/lib
    INSTALLS += target
  }
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Concurrent.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\Qt5Concurrentd.dll $${OUT_DIR} > NUL)
  }

  win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vclib
    GUID = 3f2e4bd8-734b-11e0-be72-a39b83acd183
    DEFINES += _CRT_SECURE_NO_WARNINGS
    PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}
  }
}
