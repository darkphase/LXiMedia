FREETYPE_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/freetype
include($${PWD}/freetype-version.pri)

win32-g++|macx {
  INCLUDEPATH += $${FREETYPE_DIR}/freetype-$${FREETYPE_VERSION}/include/
  LIBS += -L$${FREETYPE_DIR}/freetype-$${FREETYPE_VERSION}/objs/.libs/
}

unix:LIBS += -lfreetype
macx:LIBS += -lz
win32-g++:LIBS += -lfreetype
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(FREETYPE_DIR,/,\\)\\freetype-$${FREETYPE_VERSION}\\objs\\.libs\\libfreetype.a
