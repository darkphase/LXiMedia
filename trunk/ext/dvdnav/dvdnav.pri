DVDNAV_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/dvdnav

macx:INCLUDEPATH += $${DVDNAV_DIR}/include/
macx:LIBS += -L$${DVDNAV_DIR}/bin.macx -ldvdread

unix:LIBS += -ldvdnav

win32:include($${PWD}/../gnuwin32/gnuwin32.pri)
win32:INCLUDEPATH += $${DVDNAV_DIR}/include/
win32-g++:LIBS += -L$${DVDNAV_DIR}/bin.win32 -ldvdnav -ldvdread -ldl
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(DVDNAV_DIR,/,\\)\\bin.win32\\libdvdnav.a $$replace(DVDNAV_DIR,/,\\)\\bin.win32\\libdvdread.a $$replace(DVDNAV_DIR,/,\\)\\bin.win32\\libdl.a
