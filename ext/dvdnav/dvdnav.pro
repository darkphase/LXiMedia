TEMPLATE = subdirs
CONFIG += ordered

DVDREAD_VERSION = libdvdread-4.2.0
DVDREAD_HEADERS = $${DVDREAD_VERSION}/src/dvdread/dvd_reader.h \
 $${DVDREAD_VERSION}/src/dvdread/nav_types.h \
 $${DVDREAD_VERSION}/src/dvdread/ifo_types.h

DVDNAV_VERSION = libdvdnav-4.2.0
DVDNAV_HEADERS = $${DVDNAV_VERSION}/src/dvdnav/dvdnav.h \
 $${DVDNAV_VERSION}/src/dvdnav/dvd_types.h \
 $${DVDNAV_VERSION}/src/dvdnav/dvdnav_events.h

macx {
  system(mkdir -p $${OUT_PWD}/bin.macx)
  system(bzip2 -cdk $${PWD}/bin.macx/libdvdnav.a.bz2 > $${OUT_PWD}/bin.win32/libdvdnav.a)
  system(bzip2 -cdk $${PWD}/bin.macx/libdvdread.a.bz2 > $${OUT_PWD}/bin.win32/libdvdread.a)

  system(bzip2 -cdk $${PWD}/libdvdread_4.2.0.orig.tar.bz2 > $${OUT_PWD}/libdvdread_4.2.0.orig.tar)
  system(cd $${OUT_PWD} && tar -x -f libdvdread_4.2.0.orig.tar $${DVDREAD_HEADERS})
  system(cd $${OUT_PWD} && rm -rf include)
  system(cd $${OUT_PWD} && mv $${DVDREAD_VERSION}/src include)
  system(cd $${OUT_PWD} && rm -rf $${DVDREAD_VERSION})
  system(rm $${OUT_PWD}/libdvdread_4.2.0.orig.tar)

  system(bzip2 -cdk $${PWD}/libdvdnav_4.2.0.orig.tar.bz2 > $${OUT_PWD}/libdvdnav_4.2.0.orig.tar)
  system(cd $${OUT_PWD} && tar -x -f libdvdnav_4.2.0.orig.tar $${DVDNAV_HEADERS})
  system(cd $${OUT_PWD} && mv $${DVDNAV_VERSION}/src/dvdnav include/dvdnav)
  system(cd $${OUT_PWD} && rm -rf $${DVDNAV_VERSION})
  system(rm $${OUT_PWD}/libdvdnav_4.2.0.orig.tar)
}
win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system(mkdir $$replace(OUT_PWD,/,\\)\\bin.win32 > NUL 2>&1)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libdl.a.bz2 > $${OUT_PWD}/bin.win32/libdl.a)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libdvdnav.a.bz2 > $${OUT_PWD}/bin.win32/libdvdnav.a)
  system($${BZIP2} -cdk $${PWD}/bin.win32/libdvdread.a.bz2 > $${OUT_PWD}/bin.win32/libdvdread.a)

  system($${BZIP2} -cdk $${PWD}/libdvdread_4.2.0.orig.tar.bz2 > $${OUT_PWD}/libdvdread_4.2.0.orig.tar)
  system(cd $$replace(OUT_PWD,/,\\) && $${TAR} -x -f libdvdread_4.2.0.orig.tar $${DVDREAD_HEADERS})
  system(cd $$replace(OUT_PWD,/,\\) && del /S /Q include > NUL 2>&1 && rmdir /S /Q include > NUL 2>&1)
  system(cd $$replace(OUT_PWD,/,\\) && move $${DVDREAD_VERSION}\\src include > NUL)
  system(cd $$replace(OUT_PWD,/,\\) && rmdir /S /Q $${DVDREAD_VERSION} > NUL)
  system(del /S /Q $$replace(OUT_PWD,/,\\)\\libdvdread_4.2.0.orig.tar > NUL)

  system($${BZIP2} -cdk $${PWD}/libdvdnav_4.2.0.orig.tar.bz2 > $${OUT_PWD}/libdvdnav_4.2.0.orig.tar)
  system(cd $$replace(OUT_PWD,/,\\) && $${TAR} -x -f libdvdnav_4.2.0.orig.tar $${DVDNAV_HEADERS})
  system(cd $$replace(OUT_PWD,/,\\) && move $${DVDNAV_VERSION}\\src\\dvdnav include\\dvdnav > NUL)
  system(cd $$replace(OUT_PWD,/,\\) && rmdir /S /Q $${DVDNAV_VERSION} > NUL)
  system(del /S /Q $$replace(OUT_PWD,/,\\)\\libdvdnav_4.2.0.orig.tar > NUL)
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  system(echo // Generated from dvdnav.pro       >  include\\config.h)
}
