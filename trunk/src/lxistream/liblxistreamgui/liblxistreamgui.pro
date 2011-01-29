TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiStreamGui
include($${LXIMEDIA_DIR}/include/config.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)
include($${LXIMEDIA_DIR}/ext/exif/exif.pri)
QT += gui

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxistream
INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxistreamgui

# Files
HEADERS += $${LXIMEDIA_DIR}/include/LXiStreamGui \
    $${LXIMEDIA_DIR}/include/liblxistreamgui/sdisplay.h \
    $${LXIMEDIA_DIR}/include/liblxistreamgui/simage.h \
    $${LXIMEDIA_DIR}/include/liblxistreamgui/svideoview.h \
    $${LXIMEDIA_DIR}/include/liblxistreamgui/svumeter.h
SOURCES += sdisplay.cpp \
    simage.cpp \
    svumeter.cpp

# Platform specific
unix { 
    SOURCES += sdisplay.x11.cpp \
        svideoview.x11.cpp
    LIBS += -lXrandr \
        -lXtst \
        -lXv
    target.path = /usr/lib
    INSTALLS += target
}
win32:SOURCES += sdisplay.win.cpp \
    svideoview.win.cpp
win32-g++ { 
    system(mkdir ..\\..\\..\\bin\\ > NUL 2>&1)
    system(mkdir ..\\..\\..\\bin\\imageformats\\ > NUL 2>&1)
    release { 
        system(cp -u $$(QTDIR)/bin/QtGui4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/plugins/imageformats/qjpeg4.dll -t $${LXIMEDIA_DIR}/bin/imageformats)
        system(cp -u $$(QTDIR)/plugins/imageformats/qtiff4.dll -t $${LXIMEDIA_DIR}/bin/imageformats)
    }
}