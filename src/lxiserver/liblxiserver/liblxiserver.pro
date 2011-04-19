TEMPLATE = lib
CONFIG += dll
QT -= gui
QT += network xml
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiServer
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver

DEFINES += S_BUILD_LIBLXISERVER

# Generate/Use precompiled header
CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiServer

include($${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiServer \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/export.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/shttpclient.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/shttpengine.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/shttpserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/shttpstreamproxy.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/ssandboxclient.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/ssandboxserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/sssdpclient.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/sssdpserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/supnpbase.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/supnpconnectionmanager.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/supnpcontentdirectory.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/supnpgenaserver.h \
 $${PWD}/$${LXIMEDIA_DIR}/include/liblxiserver/supnpmediaserver.h

SOURCES += shttpclient.cpp \
 shttpengine.cpp \
 shttpengine.header.cpp \
 shttpserver.cpp \
 shttpstreamproxy.cpp \
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
    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    release {
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetwork4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXml4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtNetworkd4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
        system(cp -u $$(QTDIR)/bin/QtXmld4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vclib
    GUID = CB4BC33A-9C6C-3536-8031-5EA89E8C8C0E
    DEFINES += _CRT_SECURE_NO_WARNINGS
    PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}

    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
    release { 
        system(copy $$(QTDIR)\\bin\\QtCore4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtNetwork4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtXml4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
    }
    debug {
        system(copy $$(QTDIR)\\bin\\QtCored4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtNetworkd4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
        system(copy $$(QTDIR)\\bin\\QtXmld4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
    }
}
