# Linking the lxiserver library
include($${PWD}/../liblxicore/linklxicore.pri)
    
!contains(CONFIG, staticlib) {
  !contains(LINKED, LXiServer) {
    LINKED += LXiServer
  
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiServer.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiServer
    }
    
    win32:POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiServer.dll
    win32-g++:LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiServer
    win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\LXiServer.lib
  }
}
