TEMPLATE = subdirs
CONFIG += ordered

EXIF_VERSION = libexif-0.6.20
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

macx {
  system(mkdir -p $${OUT_PWD}/bin.macx)
  system(bzip2 -cdk $${PWD}/bin.macx/libexif.a.bz2 > $${OUT_PWD}/bin.win32/libexif.a)

  system(bzip2 -cdk $${PWD}/libexif_0.6.20.orig.tar.bz2 > $${OUT_PWD}/libexif_0.6.20.orig.tar)
  system(cd $${OUT_PWD} && tar -x -f libexif_0.6.20.orig.tar $${EXIF_HEADERS})
  system(cd $${OUT_PWD} && rm -rf include)
  system(cd $${OUT_PWD} && mv $${EXIF_VERSION} include)
  system(rm $${OUT_PWD}/libexif_0.6.20.orig.tar)
}
win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system(mkdir $$replace(OUT_PWD,/,\\)\\bin.win32 > NUL 2>&1)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libexif.a.bz2 > $${OUT_PWD}/bin.win32/libexif.a)

  system($${BZIP2} -cdk $${PWD}/libexif_0.6.20.orig.tar.bz2 > $${OUT_PWD}/libexif_0.6.20.orig.tar)
  system(cd $$replace(OUT_PWD,/,\\) && $${TAR} -x -f libexif_0.6.20.orig.tar $${EXIF_HEADERS})
  system(cd $$replace(OUT_PWD,/,\\) && del /S /Q include > NUL 2>&1)
  system(cd $$replace(OUT_PWD,/,\\) && rmdir /S /Q include > NUL 2>&1)
  system(cd $$replace(OUT_PWD,/,\\) && ren $${EXIF_VERSION} include > NUL)
  system(del /S /Q $$replace(OUT_PWD,/,\\)\\libexif_0.6.20.orig.tar > NUL 2>&1)
}
