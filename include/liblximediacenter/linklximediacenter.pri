# Linking the lximediacenter library
include($${PWD}/../liblxicore/linklxicore.pri)
include($${PWD}/../liblxiserver/linklxiserver.pri)
include($${PWD}/../liblxistream/linklxistream.pri)
include($${PWD}/../liblxistreamgui/linklxistreamgui.pri)

!contains(LINKED, LXiMediaCenter) {
  QT += sql
  LINKED += LXiMediaCenter
  
  !contains(CONFIG, staticlib) {
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiMediaCenter.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiMediaCenter
    }
    
    win32:POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiMediaCenter.dll
    win32-g++:LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiMediaCenter
    win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\LXiMediaCenter.lib
  }
}
