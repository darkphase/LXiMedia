TEMPLATE = lib
CONFIG += dll
QT -= gui
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiStream
include($${LXIMEDIA_DIR}/include/config.pri)

# Generate version.h
unix { 
    system(mkdir -p $${OBJECTS_DIR})
    system(echo \\\"$${VERSION}\\\" > $${OBJECTS_DIR}/version.h)
    INCLUDEPATH += $${OBJECTS_DIR}/
}
win32 { 
    BSOBJECTS_DIR = $$replace(OBJECTS_DIR, /, \\)
    system(if not exist $${BSOBJECTS_DIR} mkdir $${BSOBJECTS_DIR})
    system(echo \"$${VERSION}\" > $${BSOBJECTS_DIR}\\version.h)
    INCLUDEPATH += $${OBJECTS_DIR}/
}

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxistream

# Files
HEADERS += $${LXIMEDIA_DIR}/include/LXiStream \
    $${LXIMEDIA_DIR}/include/liblxistream/sanalogtuner.h \
    $${LXIMEDIA_DIR}/include/liblxistream/saudiobuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/saudiocodec.h \
    $${LXIMEDIA_DIR}/include/liblxistream/saudioformat.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sbuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sdatacodec.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sdebug.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sencodedaudiobuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sencodeddatabuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sencodedvideobuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sdigitaltuner.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sfactory.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sfactory.hpp \
    $${LXIMEDIA_DIR}/include/liblxistream/sgraph.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sinterfaces.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sinterval.h \
    $${LXIMEDIA_DIR}/include/liblxistream/smediainfo.h \
    $${LXIMEDIA_DIR}/include/liblxistream/spixels.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sserializable.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssize.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sstringparser.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssubtitlebuffer.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssubtitlefile.h \
    $${LXIMEDIA_DIR}/include/liblxistream/ssystem.h \
    $${LXIMEDIA_DIR}/include/liblxistream/staskrunner.h \
    $${LXIMEDIA_DIR}/include/liblxistream/sterminal.h \
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
    sdatacodec.cpp \
    sdebug.cpp \
    sencodedaudiobuffer.cpp \
    sencodeddatabuffer.cpp \
    sencodedvideobuffer.cpp \
    sfactory.cpp \
    sgraph.cpp \
    sinterfaces.cpp \
    sinterval.cpp \
    smediainfo.cpp \
    sserializable.cpp \
    sstringparser.cpp \
    sstringparser.iso639.cpp \
    ssubtitlebuffer.cpp \
    ssubtitlefile.cpp \
    ssystem.cpp \
    staskrunner.cpp \
    sterminal.cpp \
    stime.cpp \
    stimer.cpp \
    svideobuffer.cpp \
    svideocodec.cpp \
    svideoformat.cpp
LIBS += -lbfd \
    -liberty

# Nodes
HEADERS += $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiodecodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioencodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiomatrixnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiooutputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudioresamplenode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/saudiovideoinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sdatadecodernode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sdiscinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sfileoutputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/sioinputnode.h \
    $${LXIMEDIA_DIR}/include/liblxistream/nodes/siooutputnode.h \
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
    nodes/sdiscinputnode.cpp \
    nodes/sfileinputnode.cpp \
    nodes/sfileoutputnode.cpp \
    nodes/sioinputnode.cpp \
    nodes/siooutputnode.cpp \
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
    nodes/svideoformatconvertnode.convert.c \
    nodes/svideoformatconvertnode.demosaic.c \
    nodes/svideoformatconvertnode.unpack.c \
    nodes/svideoinputnode.cpp \
    nodes/svideoletterboxdetectnode.cpp \
    nodes/svideoresizenode.cpp

# Private classes
HEADERS += private/powermanager.h \
    private/exceptionhandler.h \
    private/log.h
SOURCES += private/powermanager.cpp \
    private/exceptionhandler.cpp \
    private/log.cpp

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
    common/rawsubtitledecoder.h
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
    common/rawsubtitledecoder.cpp

HEADERS += common/module.h
SOURCES += common/module.cpp

# Platform specific
unix {
    DEFINES += LIBCWD_THREAD_SAFE \
        CWDEBUG
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
        system(cp -u $$(QTDIR)/bin/QtOpenGL4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${LXIMEDIA_DIR}/bin)
    }
}
