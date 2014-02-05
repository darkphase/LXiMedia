TEMPLATE = subdirs
CONFIG += ordered
include($${PWD}/../../include/platform.pri)
include($${PWD}/pupnp-version.pri)

unix {
  system(mkdir -p $${OUT_PWD})

  !exists($${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/.libs/libupnp.a) {
    # Extract
    system(cp $${PWD}/libupnp-$${PUPNP_VERSION}.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf libupnp-$${PUPNP_VERSION}.tar.bz2)

    system(mkdir -p $${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/inc/upnp)
    system(cp $${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/inc/*.h $${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/inc/upnp)

    # Patch
    system(cp $${PWD}/*.patch $${OUT_PWD}/libupnp-$${PUPNP_VERSION})
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && patch -p0 < fix-ignore-specified-port.patch)

    # Compile
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && sh configure --enable-static --disable-shared --disable-samples --disable-dependency-tracking CFLAGS=\"-w $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && make)
  }
}
#win32 {
#  system(mkdir $$replace(OUT_PWD,/,\\) > NUL 2>&1)
#
#  !exists($${OUT_PWD}/pupnp_$${PUPNP_VERSION}/objs/.libs/libupnp.a) {
#    # Extract
#    system(copy /Y $$replace(PWD,/,\\)\\pupnp_$${PUPNP_VERSION}.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
#    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf pupnp_$${PUPNP_VERSION}.tar.bz2)
#
#    # Compile
#    system(echo GNUMAKE=mingw32-make ./configure --enable-static --disable-shared --disable-mmap --without-bzip2 CFLAGS=\"-w $${PLATFORM_CFLAGS}\" > $$replace(OUT_PWD,/,\\)\\freetype-$${FREETYPE_VERSION}\\bootstrap.sh)
#    system(cd $$replace(OUT_PWD,/,\\)\\pupnp_$${PUPNP_VERSION} && sh bootstrap.sh)
#    system(cd $$replace(OUT_PWD,/,\\)\\pupnp_$${PUPNP_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES})
#  }
#}
