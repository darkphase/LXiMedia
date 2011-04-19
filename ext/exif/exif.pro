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
  system(bzip2 -fdk /$$replace(PWD,:,/)/bin.win32/libexif.a.bz2)
  system(bzip2 -cdk /$$replace(PWD,:,/)/libexif_0.6.19.orig.tar.bz2 | tar -x $${EXIF_HEADERS} && rm -rf include && mv $${EXIF_VERSION} include)
}
