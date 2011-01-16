TEMPLATE = subdirs

DVDREAD_VERSION = libdvdread-4.1.3
DVDREAD_HEADERS = $${DVDREAD_VERSION}/src/dvd_reader.h \
 $${DVDREAD_VERSION}/src/nav_types.h \
 $${DVDREAD_VERSION}/src/ifo_types.h

DVDNAV_VERSION = libdvdnav-4.1.3
DVDNAV_HEADERS = $${DVDNAV_VERSION}/src/dvdnav.h \
 $${DVDNAV_VERSION}/src/dvd_types.h \
 $${DVDNAV_VERSION}/src/dvdnav_events.h

win32 {
    system(bzip2 -fdk bin.win32\\libdl.a.bz2)
    system(bzip2 -fdk bin.win32\\libdvdnav.a.bz2)
    system(bzip2 -fdk bin.win32\\libdvdread.a.bz2)
    system(bzip2 -fcdk libdvdread_4.1.3.orig.tar.bz2 | tar -x $${DVDREAD_HEADERS} && rm -rf include && mv $${DVDREAD_VERSION} include && mv include/src include/dvdread)
    system(bzip2 -fcdk libdvdnav_4.1.3.orig.tar.bz2 | tar -x $${DVDNAV_HEADERS} && cp -r $${DVDNAV_VERSION}/* include/ && rm -rf $${DVDNAV_VERSION} && mv include/src include/dvdnav)
}
