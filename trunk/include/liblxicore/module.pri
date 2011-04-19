# Module build description
TEMPLATE = lib
CONFIG += plugin
CONFIG += qt thread warn_on
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/lximedia
TARGET = $${MODULE_NAME}
INCLUDEPATH += $${PWD}/../../src/
DEPENDPATH += $${PWD}/../../src/
include($${PWD}/../config.pri)

unix {
    target.path = /usr/lib/lximedia
    INSTALLS += target
}
