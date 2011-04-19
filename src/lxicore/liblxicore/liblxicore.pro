TEMPLATE = lib
CONFIG += dll
QT -= gui
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = LXiCore
include($${LXIMEDIA_DIR}/include/config.pri)

INCLUDEPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore
DEPENDPATH += $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore

DEFINES += S_BUILD_LIBLXICORE

CONFIG += precompile_header
PRECOMPILED_HEADER = $${PWD}/$${LXIMEDIA_DIR}/include/LXiCore

# Files
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/LXiCore \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/export.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sapplication.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sdaemon.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sfactory.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/smemorypool.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/smodule.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/splatform.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sscheduler.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sscheduler.hpp \
    $${PWD}/$${LXIMEDIA_DIR}/include/liblxicore/sstringparser.h
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
    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
    system(cp -u $$(QTDIR)/bin/libgcc_s_dw2-1.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/)
    system(cp -u $$(QTDIR)/bin/mingwm10.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    release { 
        system(cp -u $$(QTDIR)/bin/QtCore4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
    debug {
        system(cp -u $$(QTDIR)/bin/QtCored4.dll -t $${OUT_PWD}/$${LXIMEDIA_DIR}/bin)
    }
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
    TEMPLATE = vclib
    GUID = 13E79835-D78E-3E44-8834-3AE6AD77D284
    DEFINES += _CRT_SECURE_NO_WARNINGS
    PRECOMPILED_SOURCE = $${PRECOMPILED_HEADER}

    system(mkdir $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL 2>&1)
    release { 
        system(copy $$(QTDIR)\\bin\\QtCore4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
    }
    debug {
        system(copy $$(QTDIR)\\bin\\QtCored4.dll $$replace(OUT_PWD,/,\\)\\..\\..\\..\\bin\\ > NUL)
    }
}
