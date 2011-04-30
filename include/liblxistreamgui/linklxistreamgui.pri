# Linking the lxistreamgui library
include($${PWD}/../liblxicore/linklxicore.pri)
include($${PWD}/../liblxistream/linklxistream.pri)

!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiStreamGui) {
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiStreamGui.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStreamGui
    }
    win32 {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiStreamGui.dll
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStreamGui
    }
  }
}
