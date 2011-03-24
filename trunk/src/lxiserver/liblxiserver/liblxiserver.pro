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
 ../../../include/liblxiserver/shttpengine.h \
 ../../../include/liblxiserver/shttpserver.h \
 ../../../include/liblxiserver/ssandboxclient.h \
 ../../../include/liblxiserver/ssandboxserver.h \
 ../../../include/liblxiserver/sssdpclient.h \
 ../../../include/liblxiserver/sssdpserver.h \
 ../../../include/liblxiserver/supnpbase.h \
 ../../../include/liblxiserver/supnpconnectionmanager.h \
 ../../../include/liblxiserver/supnpcontentdirectory.h \
 ../../../include/liblxiserver/supnpgenaserver.h \
 ../../../include/liblxiserver/supnpmediaserver.h

SOURCES += shttpengine.cpp \
 shttpengine.header.cpp \
 shttpserver.cpp \
 ssandboxclient.cpp \
 ssandboxserver.cpp \
 sssdpclient.cpp \
 sssdpserver.cpp \
 supnpbase.cpp \
 supnpconnectionmanager.cpp \
 supnpcontentdirectory.cpp \
 supnpgenaserver.cpp \
 supnpmediaserver.cpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}

win32 {
  LIBS += -lws2_32
}
