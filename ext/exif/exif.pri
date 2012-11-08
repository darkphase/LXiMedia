EXIF_DIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/ext/exif
include($${PWD}/exif-version.pri)

win32-g++|macx {
  INCLUDEPATH += $${EXIF_DIR}/libexif-$${EXIF_VERSION}/
  LIBS += -L$${EXIF_DIR}/libexif-$${EXIF_VERSION}/libexif/.libs/
}

unix:LIBS += -lexif
win32-g++:LIBS += -lexif
win32-msvc2005|win32-msvc2008|win32-msvc2010:LIBS += $$replace(EXIF_DIR,/,\\)\\libexif-$${EXIF_VERSION}\\libexif\\.libs\\libexif.a
