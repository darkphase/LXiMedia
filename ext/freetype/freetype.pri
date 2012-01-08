FREETYPE_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/freetype

unix {
  macx:INCLUDEPATH += $${FREETYPE_DIR}/include/
  macx:LIBS += -L$${FREETYPE_DIR}/bin.macx

  contains(QMAKE_HOST.os, Linux) {
    INCLUDEPATH += /usr/include/freetype2
  }

  LIBS += -lfreetype

  macx:LIBS += -lz
}

win32 {
  include($${PWD}/../gnuwin32/gnuwin32.pri)
  INCLUDEPATH += $${FREETYPE_DIR}/include/

  win32-g++:LIBS += -L$${FREETYPE_DIR}/bin.win32 -lfreetype
  win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(FREETYPE_DIR,/,\\)\\bin.win32\\libfreetype.a
}
