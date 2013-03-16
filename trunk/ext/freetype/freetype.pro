TEMPLATE = subdirs
CONFIG += ordered
include($${PWD}/../../include/platform.pri)
include($${PWD}/freetype-version.pri)

macx {
  system(mkdir -p $${OUT_PWD})

  !exists($${OUT_PWD}/freetype-$${FREETYPE_VERSION}/objs/.libs/libfreetype.a) {
    # Extract
    system(cp $${PWD}/freetype_$${FREETYPE_VERSION}.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf freetype_$${FREETYPE_VERSION}.tar.bz2)

    # Compile
    system(cd $${OUT_PWD}/freetype-$${FREETYPE_VERSION} && sh configure --enable-static --disable-shared --without-bzip2 CFLAGS=\"-w $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/freetype-$${FREETYPE_VERSION} && make)
  }
}
win32 {
  system(mkdir $$replace(OUT_PWD,/,\\) > NUL 2>&1)

  !exists($${OUT_PWD}/freetype-$${FREETYPE_VERSION}/objs/.libs/libfreetype.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\freetype_$${FREETYPE_VERSION}.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf freetype_$${FREETYPE_VERSION}.tar.bz2)

    # Compile
    system(echo GNUMAKE=mingw32-make ./configure --enable-static --disable-shared --disable-mmap --without-bzip2 CFLAGS=\"-w $${PLATFORM_CFLAGS}\" > $$replace(OUT_PWD,/,\\)\\freetype-$${FREETYPE_VERSION}\\bootstrap.sh)
    system(cd $$replace(OUT_PWD,/,\\)\\freetype-$${FREETYPE_VERSION} && sh bootstrap.sh)
    system(cd $$replace(OUT_PWD,/,\\)\\freetype-$${FREETYPE_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES})
  }
}
