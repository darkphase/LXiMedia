TEMPLATE = subdirs
CONFIG += ordered

EXIF_VERSION = libexif-0.6.19
EXIF_HEADERS = $${EXIF_VERSION}/libexif/_stdint.h \
 $${EXIF_VERSION}/libexif/exif-byte-order.h \
 $${EXIF_VERSION}/libexif/exif-content.h \
 $${EXIF_VERSION}/libexif/exif-data.h \
 $${EXIF_VERSION}/libexif/exif-data-type.h \
 $${EXIF_VERSION}/libexif/exif-entry.h \
 $${EXIF_VERSION}/libexif/exif-format.h \
 $${EXIF_VERSION}/libexif/exif-ifd.h \
 $${EXIF_VERSION}/libexif/exif-log.h \
 $${EXIF_VERSION}/libexif/exif-mem.h \
 $${EXIF_VERSION}/libexif/exif-mnote-data.h \
 $${EXIF_VERSION}/libexif/exif-tag.h \
 $${EXIF_VERSION}/libexif/exif-utils.h

win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system($${BZIP2} -fdk $${PWD}/bin.win32/libexif.a.bz2)

  system($${BZIP2} -fdk $${PWD}/libexif_0.6.19.orig.tar.bz2)
  system($${TAR} -x -f libexif_0.6.19.orig.tar $${EXIF_HEADERS})
  system(del /S /Q include > NUL 2>&1)
  system(rmdir /S /Q include > NUL 2>&1)
  system(move $${EXIF_VERSION} include > NUL 2>&1)
  system(del /S /Q libexif_0.6.19.orig.tar > NUL 2>&1)
}
