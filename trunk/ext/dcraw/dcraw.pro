TEMPLATE = subdirs
CONFIG += ordered
include($${PWD}/../../include/platform.pri)

LXIMEDIA_DIR = ../..

macx {
  DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin/LXiMediaCenter.app/Contents/MacOS

  system(mkdir -p $${OUT_PWD})

  !exists($${OUT_PWD}/dcraw-8.99.orig/dcraw) {
    # Extract
    system(cp $${PWD}/dcraw_8.99.orig.tar.bz2 $${OUT_PWD})
    system(cd $${OUT_PWD} && tar -xjf dcraw_8.99.orig.tar.bz2)

    # Compile
    system(cd $${OUT_PWD}/dcraw-8.99.orig && gcc -DNO_JPEG -DNO_LCMS -w -O3 $${PLATFORM_CFLAGS} -o dcraw dcraw.c)
  }

  system(mkdir -p $${DESTDIR})
  system(cp $${OUT_PWD}/dcraw-8.99.orig/dcraw $${DESTDIR})
}
win32 {
  DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin

  system(mkdir $$replace(OUT_PWD,/,\\) > NUL 2>&1)

  !exists($${OUT_PWD}/dcraw-8.99.orig/dcraw.exe) {
    # Extract
    system(copy /Y $$replace(PWD,/,\\)\\dcraw_8.99.orig.tar.bz2 $$replace(OUT_PWD,/,\\) > NUL)
    system(cd $$replace(OUT_PWD,/,\\) && tar -xjf dcraw_8.99.orig.tar.bz2)

    # Compile
    system(cd $$replace(OUT_PWD,/,\\)\\dcraw-8.99.orig && gcc -DNO_JPEG -DNO_LCMS -DDJGPP -w -O3 $${PLATFORM_CFLAGS} -o dcraw.exe dcraw.c -lws2_32)
  }

  system(mkdir $$replace(DESTDIR,/,\\) > NUL 2>&1)
  system(copy /Y $$replace(OUT_PWD,/,\\)\\dcraw-8.99.orig\\dcraw.exe $$replace(DESTDIR,/,\\) > NUL)
}
