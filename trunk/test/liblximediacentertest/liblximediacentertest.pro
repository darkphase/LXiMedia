TEMPLATE = app
CONFIG += qtestlib
QT += network sql xml
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lximediacentertest
INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/src/
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/exif/exif.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/ffmpeg/ffmpeg.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/fftw/fftw.pri)

unix {
  # Prevent dependency with .so files
  QMAKE_LFLAGS += $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiServer/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiMediaCenter/*.o

  POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiCore/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiServer/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStream/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiStreamGui/*.o \
    $${OUT_PWD}/$${LXIMEDIA_DIR}/obj/LXiMediaCenter/*.o
} else {
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/linklxiserver.pri)
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/linklxistreamgui.pri)
  include($${PWD}/$${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)
}

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiMediaCenter

# Files
HEADERS += commontest.h
SOURCES += main.cpp \
    commontest.cpp
LIBS += -lbfd \
    -liberty

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

# Platform specific
unix { 
    LIBS += -lbfd \
        -liberty \
        -lX11 \
        -lXext \
        -lXrandr \
        -lXtst \
        -lXv \
        -lz
    QMAKE_LFLAGS += -z \
        muldefs
}
win32 { 
    CONFIG += console
    LIBS += -lbfd \
        -liberty \
        -lws2_32
    QMAKE_LFLAGS += -Wl,-allow-multiple-definition
}
win32-g++ {
    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    release {
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtGui4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetwork4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtSql4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtTest4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtGuid4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetworkd4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtSqld4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtTestd4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
}

QMAKE_LFLAGS += $${FILES_UNDER_TEST}
POST_TARGETDEPS += $${FILES_UNDER_TEST}
