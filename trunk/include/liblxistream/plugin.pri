# Plugin build description
TEMPLATE = lib
CONFIG += plugin
CONFIG += qt thread warn_on
DESTDIR = $${LXIMEDIA_DIR}/bin/liblxistream
TARGET = $${PLUGIN_NAME}
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)

unix {
    target.path = /usr/lib/liblxistream
    INSTALLS += target
}

linux-g++|win32-g++ {
  # To make precompiled headers work properly
  QMAKE_CXXFLAGS += -UQT_PLUGIN
}
