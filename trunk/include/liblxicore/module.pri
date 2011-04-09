# Module build description
TEMPLATE = lib
CONFIG += plugin
CONFIG += qt thread warn_on
DESTDIR = $${LXIMEDIA_DIR}/bin/lximedia
TARGET = $${MODULE_NAME}
INCLUDEPATH += $${LXIMEDIA_DIR}/src/
DEPENDPATH += $${LXIMEDIA_DIR}/src/
include($${LXIMEDIA_DIR}/include/config.pri)

unix {
    target.path = /usr/lib/lximedia
    INSTALLS += target
}
