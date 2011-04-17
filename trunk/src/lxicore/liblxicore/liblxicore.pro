TEMPLATE = lib
CONFIG += dll
QT -= gui
LXIMEDIA_DIR = ../../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = LXiCore
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${LXIMEDIA_DIR}/include/liblxicore
DEPENDPATH += $${LXIMEDIA_DIR}/include/liblxicore

DEFINES += S_BUILD_LIBLXICORE

# Generate/Use precompiled header
CONFIG += precompile_header
PRECOMPILED_HEADER = $${LXIMEDIA_DIR}/include/LXiCore

# Files
HEADERS += $${LXIMEDIA_DIR}/include/LXiCore \
    $${LXIMEDIA_DIR}/include/liblxicore/export.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sapplication.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sdaemon.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sfactory.h \
    $${LXIMEDIA_DIR}/include/liblxicore/smemorypool.h \
    $${LXIMEDIA_DIR}/include/liblxicore/smodule.h \
    $${LXIMEDIA_DIR}/include/liblxicore/splatform.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sscheduler.h \
    $${LXIMEDIA_DIR}/include/liblxicore/sscheduler.hpp \
    $${LXIMEDIA_DIR}/include/liblxicore/sstringparser.h
SOURCES += sapplication.cpp \
    sapplication.exchandler.cpp \
    sapplication.log.cpp \
    sfactory.cpp \
    smemorypool.cpp \
    smodule.cpp \
    sscheduler.cpp \
    sstringparser.cpp \
    sstringparser.iso639.cpp

# Platform specific
linux-g++|win32-g++ {
  LIBS += -lbfd -liberty
}
unix {
    SOURCES += sdaemon.unix.cpp

    target.path = /usr/lib
    INSTALLS += target
}
win32 {
    SOURCES += sdaemon.win.cpp
}
win32-g++ { 
    system(mkdir ..\\..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${LXIMEDIA_DIR}/bin)
    release { 
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${LXIMEDIA_DIR}/bin)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${LXIMEDIA_DIR}/bin)
    }
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vclib
    GUID = 13E79835-D78E-3E44-8834-3AE6AD77D284
    DEFINES += _CRT_SECURE_NO_WARNINGS
    PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}

    system(mkdir ..\\..\\..\\bin\\ > NUL 2>&1)
    release { 
        system(copy $$(QTDIR)\\bin\\QtCore4.dll ..\\..\\..\\bin\\ > NUL)
    }
    debug {
        system(copy $$(QTDIR)\\bin\\QtCored4.dll ..\\..\\..\\bin\\ > NUL)
    }
}
