TEMPLATE = lib
CONFIG += dll
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
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sanalogtuner.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/saudiobuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/saudiocodec.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/saudioformat.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sbuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sdatabuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sdatacodec.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sencodedaudiobuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sencodeddatabuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sencodedvideobuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sdigitaltuner.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sgraph.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sinterfaces.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/sinterval.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/smediainfo.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/spixels.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/ssize.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/ssubpicturebuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/ssubtitlebuffer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/ssubtitlefile.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/stime.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/stimer.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/stuner.h \
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
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiodecodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioencodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiomatrixnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiooutputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioresamplenode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiovideoinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sdatadecodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileoutputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/sioinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/siooutputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/splaylistnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/ssubpicturerendernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/ssubtitlerendernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/stimestampresamplernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/stimestampsyncnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoboxnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideodecodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideodeinterlacenode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoencodernode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoformatconvertnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoinputnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoletterboxdetectnode.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/nodes/svideoresizenode.h
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
    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    release { 
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtGui4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtGuid4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vclib
    GUID = 6AF51170-3E95-3E73-BFC4-0BBD8E57F07B
    DEFINES += _CRT_SECURE_NO_WARNINGS
    PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}

    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
    release { 
        system(copy $$(QTDIR)\\bin\\QtCore4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtGui4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtXml4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
    }
    debug {
        system(copy $$(QTDIR)\\bin\\QtCored4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtGuid4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtXmld4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
    }
}
