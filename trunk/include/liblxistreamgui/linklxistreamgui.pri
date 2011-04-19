# Linking the lxistreamgui library
include($${PWD}/../liblxicore/linklxicore.pri)
include($${PWD}/../liblxistream/linklxistream.pri)

LXISTREAM_VERSION_MAJOR = $$system(cat ../../VERSION)
LXISTREAM_VERSION_MAJOR ~= s/\\.[0-9]+.+/

!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiStreamGui) {
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiStreamGui.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStreamGui
    }

    win32-g++ {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiStreamGui$${LXISTREAM_VERSION_MAJOR}.dll
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStreamGui$${LXISTREAM_VERSION_MAJOR}
    }
  }
}
