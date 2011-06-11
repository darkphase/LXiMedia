# Linking the lxistreamgui library
include($${PWD}/../liblxicore/linklxicore.pri)
include($${PWD}/../liblxistream/linklxistream.pri)

!contains(CONFIG, staticlib) {
  !contains(LINKED, LXiStreamGui) {
    LINKED += LXiStreamGui
  
    unix|win32-g++:LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStreamGui
    win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\LXiStreamGui.lib
  }
}