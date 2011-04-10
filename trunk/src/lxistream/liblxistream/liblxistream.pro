TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiStream
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxistream
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblxistream

DEFINES += S_BUILD_LIBLXISTREAM

linux-g++|win32-g++ {
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiStream
}

include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)

# Files
HEADERS += $${LXIMEDIA_DIR}/include/LXiStream \
    $${LXIMEDIA_DIR}/include/liblxistream/export.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sanalogtuner.h \
    $${LXIMEDIA_DIR}/include/liblxistream/saudiobuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/saudiocodec.h \
    $${LXIMEDIA_DIR}/include/liblxistream/saudioformat.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sbuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sdatabuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sdatacodec.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sencodedaudiobuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sencodeddatabuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sencodedvideobuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sdigitaltuner.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sgraph.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sinterfaces.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sinterval.h \
    $${LXIMEDIA_DIR}/include/liblxistream/smediainfo.h \
    $${LXIMEDIA_DIR}/include/liblxistream/spixels.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssize.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssubpicturebuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssubtitlebuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssubtitlefile.h \
    $${LXIMEDIA_DIR}/include/liblxistream/stime.h \
    $${LXIMEDIA_DIR}/include/liblxistream/stimer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/stuner.h \
    $${LXIMEDIA_DIR}/include/liblxistream/svideobuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/svideocodec.h \
    $${LXIMEDIA_DIR}/include/liblxistream/svideoformat.h
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
    smediainfo.cpp \
    ssubpicturebuffer.cpp \
    ssubtitlebuffer.cpp \
    ssubtitlefile.cpp \
    spixels.cpp \
    spixels.convert.c \
    stime.cpp \
    stimer.cpp \
    svideobuffer.cpp \
    svideocodec.cpp \
    svideoformat.cpp

# Nodes
HEADERS += $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiodecodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioencodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiomatrixnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiooutputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioresamplenode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiovideoinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sdatadecodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileoutputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sioinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/siooutputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/splaylistnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/ssubpicturerendernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/ssubtitlerendernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/stimestampresamplernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/stimestampsyncnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoboxnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideodecodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideodeinterlacenode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoencodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoformatconvertnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoletterboxdetectnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoresizenode.h
SOURCES += nodes/saudiodecodernode.cpp \
    nodes/saudioencodernode.cpp \
    nodes/saudioinputnode.cpp \
    nodes/saudiomatrixnode.cpp \
    nodes/saudiomatrixnode.mix.c \
    nodes/saudiooutputnode.cpp \
    nodes/saudioresamplenode.cpp \
    nodes/saudiovideoinputnode.cpp \
    nodes/sdatadecodernode.cpp \
    nodes/sfileinputnode.cpp \
    nodes/sfileoutputnode.cpp \
    nodes/sioinputnode.cpp \
    nodes/siooutputnode.cpp \
    nodes/splaylistnode.cpp \
    nodes/ssubpicturerendernode.cpp \
    nodes/ssubpicturerendernode.mix.c \
    nodes/ssubtitlerendernode.cpp \
    nodes/ssubtitlerendernode.mix.c \
    nodes/stimestampresamplernode.cpp \
    nodes/stimestampsyncnode.cpp \
    nodes/svideoboxnode.cpp \
    nodes/svideoboxnode.box.c \
    nodes/svideodecodernode.cpp \
    nodes/svideodeinterlacenode.cpp \
    nodes/svideoencodernode.cpp \
    nodes/svideoformatconvertnode.cpp \
    nodes/svideoinputnode.cpp \
    nodes/svideoletterboxdetectnode.cpp \
    nodes/svideoresizenode.cpp

HEADERS += lxistreamprivate.h
SOURCES += lxistreamprivate.cpp

# Common backend classes
HEADERS += common/audiooutput.h \
    common/audioresampler.h \
    common/deinterlace.h \
    common/formatprober.h \
    common/mpeg.h \
    common/pcmaudiodecoder.h \
    common/pcmaudioencoder.h \
    common/psbufferreader.h \
    common/psbufferwriter.h \
    common/rawsubtitledecoder.h \
    common/videoformatconverter.h
SOURCES += common/audiooutput.cpp \
    common/audioresampler.cpp \
    common/audioresampler.resample.c \
    common/deinterlace.cpp \
    common/deinterlace.mix.c \
    common/formatprober.cpp \
    common/mpeg.cpp \
    common/pcmaudiodecoder.cpp \
    common/pcmaudioencoder.cpp \
    common/pcmaudio.swap.c \
    common/psbufferreader.cpp \
    common/psbufferwriter.cpp \
    common/rawsubtitledecoder.cpp \
    common/videoformatconverter.cpp \
    common/videoformatconverter.convert.c \
    common/videoformatconverter.demosaic.c \
    common/videoformatconverter.unpack.c

HEADERS += common/module.h
SOURCES += common/module.cpp

# Platform specific
unix {
    target.path = /usr/lib
    INSTALLS += target
}
win32-g++ { 
    system(mkdir ..\\..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${LXIMEDIA_DIR}/bin)
    release { 
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetwork4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${LXIMEDIA_DIR}/bin)
    }
}