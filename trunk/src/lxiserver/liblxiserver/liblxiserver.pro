TEMPLATE = lib
CONFIG += dll
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiServer
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxiserver
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblxiserver

linux-g++|win32-g++ {
  # Generate/Use precompiled header
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiServer
}

# Files
HEADERS += ../../../include/LXiServer \
 ../../../include/liblxiserver/httpserver.h \
 ../../../include/liblxiserver/ssdpclient.h \
 ../../../include/liblxiserver/ssdpserver.h \
 ../../../include/liblxiserver/upnpbase.h \
 ../../../include/liblxiserver/upnpconnectionmanager.h \
 ../../../include/liblxiserver/upnpcontentdirectory.h \
 ../../../include/liblxiserver/upnpmediaserver.h

SOURCES += httpserver.cpp \
 httpserver.header.cpp \
 ssdpclient.cpp \
 ssdpserver.cpp \
 upnpbase.cpp \
 upnpconnectionmanager.cpp \
 upnpcontentdirectory.cpp \
 upnpmediaserver.cpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32 {
  LIBS += -lws2_32
}
