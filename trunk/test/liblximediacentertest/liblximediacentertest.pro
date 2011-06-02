TEMPLATE = app
CONFIG += qtestlib
macx:CONFIG -= app_bundle
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

# Files
HEADERS += commontest.h
SOURCES += main.cpp \
    commontest.cpp

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
win32:QMAKE_POST_LINK = $${DESTDIR}/$${TARGET} -silent

# Platform specific
unix { 
    LIBS += -lz
    !macx {
        LIBS += -lX11 \
            -lXext \
            -lXrandr \
            -lXtst \
            -lXv
    }
}
win32 { 
    CONFIG += console
    LIBS += -lbfd \
        -liberty \
        -lws2_32
    QMAKE_LFLAGS += -Wl,-allow-multiple-definition
}

# Windows specific
win32 {
  OUT_DIR = $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin

  system(mkdir $${OUT_DIR} > NUL 2>&1)
  release {
    system(copy /Y $$(QTDIR)\\bin\\QtTest4.dll $${OUT_DIR} > NUL)
  }
  debug {
    system(copy /Y $$(QTDIR)\\bin\\QtTestd4.dll $${OUT_DIR} > NUL)
  }
}
