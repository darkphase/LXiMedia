# Linking the lxiserver library
include($${PWD}/../liblxicore/linklxicore.pri)
    
!contains(LINKED, LXiServer) {
  LINKED += LXiServer

  QT += xml
  
  !contains(CONFIG, staticlib) {
    unix|win32-g++:LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiServer
    win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\LXiServer.lib
  }
}
