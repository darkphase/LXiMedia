# Linking the lxistreamgl library
include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

LXISTREAM_VERSION_MAJOR = $$system(cat ../../VERSION)
LXISTREAM_VERSION_MAJOR ~= s/\.[0-9]+.+/

!contains(INCLUDEPATH, $${LXIMEDIA_DIR}/include/) {
  INCLUDEPATH += $${LXIMEDIA_DIR}/include/
  DEPENDPATH += $${LXIMEDIA_DIR}/include/
}

!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiStreamGl) {
    unix {
      TARGETDEPS += $${LXIMEDIA_DIR}/bin/libLXiStreamGl.so
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiStreamGl
    }

    win32-g++ {
      TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiStreamGl$${LXISTREAM_VERSION_MAJOR}.dll
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiStreamGl$${LXISTREAM_VERSION_MAJOR}
    }
  }
}
