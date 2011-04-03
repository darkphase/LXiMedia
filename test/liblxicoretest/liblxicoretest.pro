TEMPLATE = app
CONFIG += qtestlib
QT += network \
    xml
LXIMEDIA_DIR = ../..
DESTDIR = $${LXIMEDIA_DIR}/bin
TARGET = lxicoretest
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)

# Files
HEADERS += coretest.h
SOURCES += main.cpp \
    coretest.cpp
LIBS += -lbfd \
    -liberty

# Run tests after link
unix:QMAKE_POST_LINK = $(TARGET) -silent
#win32:QMAKE_POST_LINK = $(TARGET) -silent
# Prevent dependency with .so files
FILES_UNDER_TEST = $${LXIMEDIA_DIR}/obj/LXiCore/*.o

# Platform specific
unix {
    QMAKE_LFLAGS += -z \
        muldefs
}
win32 { 
    CONFIG += console
}
QMAKE_LFLAGS += $${FILES_UNDER_TEST}
POST_TARGETDEPS += $${FILES_UNDER_TEST}
