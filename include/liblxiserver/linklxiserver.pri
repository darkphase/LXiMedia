# Linking the lxiserver library
include($${PWD}/../liblxicore/linklxicore.pri)
    
!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiServer) {
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiServer.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiServer
    }
    win32-g++ {
      LXISERVER_VERSION_MAJOR = $$system(cat ../../VERSION)
      LXISERVER_VERSION_MAJOR ~= s/\\.[0-9]+.+/
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiServer$${LXISERVER_VERSION_MAJOR}.dll
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiServer$${LXISERVER_VERSION_MAJOR}
    }
    win32-msvc2005|win32-msvc2008|win32-msvc2010 {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiCore.dll
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiCore
    }
  }
}
