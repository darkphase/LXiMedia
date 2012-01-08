TEMPLATE = subdirs
CONFIG += ordered

LXIMEDIA_DIR = ../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin

macx {
  system(mkdir -p $${DESTDIR})
  system(bzip2 -cdk $${PWD}/bin.macx/dcraw.bz2 > $${DESTDIR}/dcraw)
  system(chmod a+x $${DESTDIR}/dcraw)
}
win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe

  system(mkdir $$replace(DESTDIR,/,\\) > NUL 2>&1)
  system($${BZIP2} -cdk $${PWD}/bin.win32/dcraw.exe.bz2 > $${DESTDIR}/dcraw.exe)
}
