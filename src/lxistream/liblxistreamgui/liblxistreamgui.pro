TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiStreamGui
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)
include($${PWD}/$${LXIMEDIA_DIR}/ext/exif/exif.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxistream $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui

DEFINES += S_BUILD_LIBLXISTREAMGUI

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiStreamGui

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${PWD}/$${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiStreamGui \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/export.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/sdisplay.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/simage.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/svideoview.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxistreamgui/svumeter.h
SOURCES += sdisplay.cpp \
    simage.cpp \
    svumeter.cpp

# Platform specific
unix { 
    SOURCES += sdisplay.x11.cpp \
        svideoview.x11.cpp
    LIBS += -lX11 \
        -lXext \
        -lXrandr \
        -lXtst \
        -lXv
    target.path = /usr/lib
    INSTALLS += target
}
win32:SOURCES += sdisplay.win.cpp \
    svideoview.win.cpp
win32-g++ {
    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\imageformats\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    release {
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtGui4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/plugins/imageformats/qjpeg4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/imageformats)
        system(cp -u $$(QTDIR)/plugins/imageformats/qtiff4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/imageformats)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtGuid4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/plugins/imageformats/qjpegd4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/imageformats)
        system(cp -u $$(QTDIR)/plugins/imageformats/qtiffd4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/imageformats)
    }
}
