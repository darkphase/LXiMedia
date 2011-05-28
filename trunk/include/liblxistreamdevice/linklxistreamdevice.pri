# Linking the lxistreamdevice library
include($${PWD}/../liblxicore/linklxicore.pri)
include($${PWD}/../liblxistream/linklxistream.pri)

!contains(CONFIG, staticlib) {
  !contains(LINKED, LXiStreamDevice) {
    LINKED += LXiStreamDevice
  
    unix {
      POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/libLXiStreamDevice.so
      LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStreamDevice
    }
    
    win32:POST_TARGETDEPS += $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiStreamDevice.dll
    win32-g++:LIBS += -L$${OUT_PWD}/$${LXIMEDIA_DIR}/bin -lLXiStreamDevice
    win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(OUT_PWD,/,\\)\\$$replace(LXIMEDIA_DIR,/,\\)\\bin\\LXiStreamDevice.lib
  }
}
