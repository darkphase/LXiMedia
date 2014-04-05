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

    # Patch
    system(cp $${PWD}/*.patch $${OUT_PWD}/libupnp-$${PUPNP_VERSION})
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && patch -p0 < fix-binding-port.patch)
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && patch -p0 < fix-repeat-announcement-spread.patch)
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && patch -p0 < add-support-cachecontrol-nocache.patch)
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && patch -p0 < add-support-multiple-interfaces.patch)

    # Compile
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && sh configure --enable-static --disable-shared --disable-samples --disable-dependency-tracking CFLAGS=\"-w -fPIC $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/libupnp-$${PUPNP_VERSION} && make)

    # Public headers
    system(mkdir -p $${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/inc/upnp)
    system(cp $${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/inc/*.h $${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/inc/upnp)
  }
}
win32 {
  system(mkdir $$replace(OUT_PWD,/,\\) > NUL 2>&1)

  !exists($${OUT_PWD}/libupnp-$${PUPNP_VERSION}/upnp/.libs/libupnp.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\libupnp-$${PUPNP_VERSION}.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf libupnp-$${PUPNP_VERSION}.tar.bz2)

    # Patch
    system(copy /Y $$replace(PWD,/,\\)\\*.patch $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} > NUL)
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && patch -p0 < fix-mingw47-build.patch)
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && patch -p0 < fix-windows-xp-compatibility.patch)
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && patch -p0 < fix-binding-port.patch)
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && patch -p0 < fix-repeat-announcement-spread.patch)
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && patch -p0 < add-support-cachecontrol-nocache.patch)
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && patch -p0 < add-support-multiple-interfaces.patch)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && sh configure --enable-static --disable-shared --disable-samples --disable-dependency-tracking CFLAGS=\"-w -fPIC -DUPNP_STATIC_LIB $${PLATFORM_CFLAGS}\")
    system(cd $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION} && mingw32-make MAKE=mingw32-make -j $${PLATFORM_NUMCORES})

    # Public headers
    system(mkdir $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION}\\upnp\\inc\\upnp)
    system(copy /Y $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION}\\upnp\\inc\\*.h $$replace(OUT_PWD,/,\\)\\libupnp-$${PUPNP_VERSION}\\upnp\\inc\\upnp > NUL)
  }
}
