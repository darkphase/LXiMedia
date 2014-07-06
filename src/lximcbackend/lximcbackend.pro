TEMPLATE = app
CONFIG -= qt
LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = lximcbackend2

QMAKE_CXXFLAGS += -std=c++11 -pthread
QMAKE_LIBS += -pthread -lpthread

include($${PWD}/$${LXIMEDIA_DIR}/ext/pupnp/pupnp.pri)

#QMAKE_CXXFLAGS += -fpermissive -I/usr/include/gtk-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/pango-1.0 -I/usr/include/gio-unix-2.0/ -I/usr/include/freetype2 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/pixman-1 -I/usr/include/libpng12 -I/usr/include/harfbuzz
#LIBS += -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgio-2.0 -lpangoft2-1.0 -lpangocairo-1.0 -lgdk_pixbuf-2.0 -lcairo -lpango-1.0 -lfontconfig -lgobject-2.0 -lglib-2.0 -lfreetype

# Files
SOURCES += \
    messageloop.cpp \
    upnp.cpp \
    ixml_structures.cpp \
    backend.cpp \
    rootdevice.cpp \
    main.cpp \
    connection_manager.cpp \
    content_directory.cpp
HEADERS += \
    messageloop.h \
    upnp.h \
    ixml_structures.h \
    backend.h \
    rootdevice.h \
    connection_manager.h \
    content_directory.h

LIBS += -lvlc

# Platform specific
unix { 
  !macx {
    target.path = /usr/bin
    INSTALLS += target
  }
}
win32 { 
  CONFIG += console
}
