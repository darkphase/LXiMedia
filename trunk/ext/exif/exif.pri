EXIF_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/exif

macx:INCLUDEPATH += $${EXIF_DIR}/include/
macx:LIBS += -L$${EXIF_DIR}/bin.macx

unix:LIBS += -lexif

win32:include($${PWD}/../gnuwin32/gnuwin32.pri)
win32:INCLUDEPATH += $${EXIF_DIR}/include/
win32-g++:LIBS += -L$${EXIF_DIR}/bin.win32 -lexif
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(EXIF_DIR,/,\\)\\bin.win32\\libexif.a
