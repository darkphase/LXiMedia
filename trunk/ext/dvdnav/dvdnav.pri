DVDNAV_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/dvdnav
include($${PWD}/dvdnav-version.pri)

win32-g++|macx {
  INCLUDEPATH += \
    $${DVDNAV_DIR}/libdvdread-$${DVDREAD_VERSION}/src/ \
    $${DVDNAV_DIR}/libdvdnav-$${DVDNAV_VERSION}/src/

  LIBS += \
    -L$${DVDNAV_DIR}/libdvdread-$${DVDREAD_VERSION}/obj/ \
    -L$${DVDNAV_DIR}/libdvdnav-$${DVDNAV_VERSION}/obj/
}

macx:LIBS += -ldvdread
unix:LIBS += -ldvdnav
win32-g++:LIBS += -L$${DVDNAV_DIR}/dlfcn-win32-$${DLFCN_VERSION}/ -ldvdnav -ldvdread -ldl

win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += \
  $$replace(DVDNAV_DIR,/,\\)\\dlfcn-win32-$${DLFCN_VERSION}\\libdl.a \
  $$replace(DVDNAV_DIR,/,\\)\\libdvdread-$${DVDREAD_VERSION}\\obj\\libdvdread.a \
  $$replace(DVDNAV_DIR,/,\\)\\libdvdread-$${DVDREAD_VERSION}\\obj\\libdvdread.a
