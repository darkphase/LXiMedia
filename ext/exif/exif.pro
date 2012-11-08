TEMPLATE = subdirs
CONFIG += ordered
include($${PWD}/../../include/platform.pri)
include($${PWD}/exif-version.pri)

macx {
  system(mkdir -p $${OUT_PWD})

  !exists($${OUT_PWD}/libexif-$${EXIF_VERSION}/libexif/.libs/libexif.a) {
    # Extract
    system(cp $${PWD}/libexif_$${EXIF_VERSION}.orig.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf libexif_$${EXIF_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $${OUT_PWD}/libexif-$${EXIF_VERSION} && sh configure --enable-static --disable-shared CFLAGS=\"-w -O2 $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/libexif-$${EXIF_VERSION} && make -j $${PLATFORM_NUMCORES})
  }
}
win32 {
  system(mkdir $$replace(OUT_PWD,/,\\) > NUL 2>&1)

  !exists($${OUT_PWD}/libexif-$${EXIF_VERSION}/libexif/.libs/libexif.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\libexif_$${EXIF_VERSION}.orig.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf libexif_$${EXIF_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\libexif-$${EXIF_VERSION} && sh configure --enable-static --disable-shared CFLAGS=\"-w -O2 $${PLATFORM_CFLAGS}")
    system(cd $$replace(OUT_PWD,/,\\)\\libexif-$${EXIF_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES} MAKE=mingw32-make)
  }
}
