# Linking the lxistream library
LXISERVER_VERSION_MAJOR = $$system(cat ../../VERSION)
LXISERVER_VERSION_MAJOR ~= s/\\.[0-9]+.+/

!contains(INCLUDEPATH, $${LXIMEDIA_DIR}/include/) {
  INCLUDEPATH += $${LXIMEDIA_DIR}/include/
  DEPENDPATH += $${LXIMEDIA_DIR}/include/
}
    
!contains(CONFIG, staticlib) {
  !contains(LIBS, -lLXiServer) {
    unix {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/libLXiServer.so
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiServer
    }

    win32-g++ {
      POST_TARGETDEPS += $${LXIMEDIA_DIR}/bin/LXiServer$${LXISERVER_VERSION_MAJOR}.dll
      LIBS += -L$${LXIMEDIA_DIR}/bin -lLXiServer$${LXISERVER_VERSION_MAJOR}
    }
  }
}
