TEMPLATE = subdirs
CONFIG += ordered

DVDREAD_VERSION = libdvdread-4.1.3
DVDREAD_HEADERS = $${DVDREAD_VERSION}/src/dvd_reader.h \
 $${DVDREAD_VERSION}/src/nav_types.h \
 $${DVDREAD_VERSION}/src/ifo_types.h

DVDNAV_VERSION = libdvdnav-4.1.3
DVDNAV_HEADERS = $${DVDNAV_VERSION}/src/dvdnav.h \
 $${DVDNAV_VERSION}/src/dvd_types.h \
 $${DVDNAV_VERSION}/src/dvdnav_events.h

macx {
  system(bzip2 -fdk $${PWD}/bin.macx/libdvdnav.a.bz2)
  system(bzip2 -fdk $${PWD}/bin.macx/libdvdread.a.bz2)

  system(bzip2 -fdk $${PWD}/libdvdread_4.1.3.orig.tar.bz2)
  system(tar -x -f libdvdread_4.1.3.orig.tar $${DVDREAD_HEADERS})
  system(rm -rf include)
  system(mv $${DVDREAD_VERSION} include)
  system(mv include/src include/dvdread)
  system(rm libdvdread_4.1.3.orig.tar)

  system(bzip2 -fdk $${PWD}/libdvdnav_4.1.3.orig.tar.bz2)
  system(tar -x -f libdvdnav_4.1.3.orig.tar $${DVDNAV_HEADERS})
  system(mv $${DVDNAV_VERSION}/src include/dvdnav)
  system(rm -rf $${DVDNAV_VERSION})
  system(rm libdvdnav_4.1.3.orig.tar)
}
win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system($${BZIP2} -fdk $${PWD}/bin.win32/libdl.a.bz2)
  system($${BZIP2} -fdk $${PWD}/bin.win32/libdvdnav.a.bz2)
  system($${BZIP2} -fdk $${PWD}/bin.win32/libdvdread.a.bz2)

  system($${BZIP2} -fdk $${PWD}/libdvdread_4.1.3.orig.tar.bz2)
  system($${TAR} -x -f libdvdread_4.1.3.orig.tar $${DVDREAD_HEADERS})
  system(del /S /Q include > NUL 2>&1)
  system(rmdir /S /Q include > NUL 2>&1)
  system(ren $${DVDREAD_VERSION} include > NUL)
  system(move include\\src include\\dvdread > NUL)
  system(del /S /Q libdvdread_4.1.3.orig.tar > NUL)

  system($${BZIP2} -fdk $${PWD}/libdvdnav_4.1.3.orig.tar.bz2)
  system($${TAR} -x -f libdvdnav_4.1.3.orig.tar $${DVDNAV_HEADERS})
  system(move $${DVDNAV_VERSION}\\src include\\dvdnav > NUL)
  system(del /S /Q $${DVDNAV_VERSION} > NUL)
  system(rmdir /S /Q $${DVDNAV_VERSION} > NUL)
  system(del /S /Q libdvdnav_4.1.3.orig.tar > NUL)
}
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  system(echo // Generated from dvdnav.pro       >  include\\config.h)
}
