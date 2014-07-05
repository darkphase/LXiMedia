TEMPLATE = lib
CONFIG += plugin
CONFIG += qt thread warn_on

win32 {
    DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/lximedia
} else {
    LXIMEDIA_VERSION_MAJOR = $$system(cat $${PWD}/../../VERSION)
    LXIMEDIA_VERSION_MAJOR ~= s/\\.[0-9]+.+/
    macx {
        DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiMediaCenter.app/Contents/PlugIns/lximedia$${LXIMEDIA_VERSION_MAJOR}
    } else {
        DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/lximedia$${LXIMEDIA_VERSION_MAJOR}
    }
}

TARGET = $${MODULE_NAME}
INCLUDEPATH += $${PWD}/../../src/
DEPENDPATH += $${PWD}/../../src/
include($${PWD}/../config.pri)

unix {
    !macx {
        target.path = /usr/lib/lximedia0
        INSTALLS += target
    }
}
