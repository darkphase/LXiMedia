TEMPLATE = lib
CONFIG += dll
QT -= gui
QT += network xml
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiServer
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxiserver
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblxiserver

DEFINES += S_BUILD_LIBLXISERVER

linux-g++|win32-g++ {
  # Generate/Use precompiled header
  CONFIG += precompile_header
  PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiServer
}

include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)

# Files
HEADERS += ../../../include/LXiServer \
 ../../../include/liblxiserver/export.h \
 ../../../include/liblxiserver/shttpclient.h \
 ../../../include/liblxiserver/shttpengine.h \
 ../../../include/liblxiserver/shttpproxy.h \
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

SOURCES += shttpclient.cpp \
 shttpengine.cpp \
 shttpengine.header.cpp \
 shttpproxy.cpp \
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

HEADERS += lxiserverprivate.h
SOURCES += lxiserverprivate.cpp

unix {
    target.path = /usr/lib
    INSTALLS += target
}
win32 {
  LIBS += -lws2_32
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
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetworkd4.dll -t $${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${LXIMEDIA_DIR}/bin)
    }
}
