TEMPLATE = subdirs
CONFIG += ordered
include($${PWD}/../../include/platform.pri)
include($${PWD}/dvdnav-version.pri)

macx {
  system(mkdir -p $${OUT_PWD})

  !exists($${OUT_PWD}/libdvdread-$${DVDREAD_VERSION}/obj/libdvdread.a) {
    # Extract
    system(cp $${PWD}/libdvdread_$${DVDREAD_VERSION}.orig.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf libdvdread_$${DVDREAD_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $${OUT_PWD}/libdvdread-$${DVDREAD_VERSION} && sh configure2 --enable-static --disable-shared --disable-debug --extra-cflags=\"-w -I../../dlfcn-win32-r19/ -O2 $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/libdvdread-$${DVDREAD_VERSION} && make)
  }

  !exists($${OUT_PWD}/libdvdnav-$${DVDNAV_VERSION}/obj/libdvdnav.a) {
    # Extract
    system(cp $${PWD}/libdvdnav_$${DVDNAV_VERSION}.orig.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf libdvdnav_$${DVDNAV_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $${OUT_PWD}/libdvdnav-$${DVDNAV_VERSION} && patch -i $${PWD}/libdvdnav_$${DVDNAV_VERSION}.orig.patch)
    system(cd $${OUT_PWD}/libdvdnav-$${DVDNAV_VERSION} && sh configure2 --enable-static --disable-shared --disable-debug --extra-cflags=\"-w -I../../libdvdread-$${DVDNAV_VERSION}/src/ -O2 $${PLATFORM_CFLAGS}\")
    system(cd $${OUT_PWD}/libdvdnav-$${DVDNAV_VERSION} && make)
  }
}
win32 {
  system(mkdir $$replace(OUT_PWD,/,\\) > NUL 2>&1)

  !exists($${OUT_PWD}/dlfcn-win32-$${DLFCN_VERSION}/libdl.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\dlfcn-win32-$${DLFCN_VERSION}.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf dlfcn-win32-$${DLFCN_VERSION}.tar.bz2)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\dlfcn-win32-$${DLFCN_VERSION} && sh configure > NUL)
    system(cd $$replace(OUT_PWD,/,\\)\\dlfcn-win32-$${DLFCN_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES})
  }

  !exists($${OUT_PWD}/libdvdread-$${DVDREAD_VERSION}/obj/libdvdread.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\libdvdread_$${DVDREAD_VERSION}.orig.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf libdvdread_$${DVDREAD_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\libdvdread-$${DVDREAD_VERSION} && sh configure2 --enable-static --disable-shared --disable-debug --extra-cflags=\"-w -I../../dlfcn-win32-r19/ -O2 $${PLATFORM_CFLAGS}\")
    system(cd $$replace(OUT_PWD,/,\\)\\libdvdread-$${DVDREAD_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES})
  }

  !exists($${OUT_PWD}/libdvdnav-$${DVDNAV_VERSION}/obj/libdvdnav.a) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\libdvdnav_$${DVDNAV_VERSION}.orig.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf libdvdnav_$${DVDNAV_VERSION}.orig.tar.bz2)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\libdvdnav-$${DVDNAV_VERSION} && patch -i $${PWD}/libdvdnav_$${DVDNAV_VERSION}.orig.patch)
    system(cd $$replace(OUT_PWD,/,\\)\\libdvdnav-$${DVDNAV_VERSION} && sh configure2 --enable-static --disable-shared --disable-debug --extra-cflags=\"-w -I../../libdvdread-$${DVDNAV_VERSION}/src/ -O2 $${PLATFORM_CFLAGS}\")
    system(cd $$replace(OUT_PWD,/,\\)\\libdvdnav-$${DVDNAV_VERSION} && mingw32-make -j $${PLATFORM_NUMCORES})
  }
}
