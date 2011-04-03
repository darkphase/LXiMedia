# Linking the lxistreamgui library
include($${LXIMEDIA_DIR}/include/liblxicore/linklxicore.pri)
include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

LXISTREAM_VERSION_MAJOR = $$system(cat ../../VERSION)
LXISTREAM_VERSION_MAJOR ~= s/\\.[0-9]+.+/

!contains(INCLUDEPATH, $${LXIMEDIA_DIR}/include/) {
  INCLUDEPATH += $${LXIMEDIA_DIR}/include/
  DEPENDPATH += $${LXIMEDIA_DIR}/include/
}

!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiStreamGui) {
    unix {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/libLXiStreamGui.so
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiStreamGui
    }

    win32-g++ {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiStreamGui$${LXISTREAM_VERSION_MAJOR}.dll
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiStreamGui$${LXISTREAM_VERSION_MAJOR}
    }
  }
}
