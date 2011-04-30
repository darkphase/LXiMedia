TEMPLATE = subdirs
CONFIG += ordered

FFTW_VERSION = fftw-3.2.2
FFTW_HEADERS = $${FFTW_VERSION}/api/fftw3.h

win32 {
  BZIP2 = $$replace(PWD,/,\\)\\..\\gnuwin32\\bzip2.exe
  TAR = $$replace(PWD,/,\\)\\..\\gnuwin32\\tar.exe

  system($${BZIP2} -fdk $${PWD}/bin.win32/libfftw3f.a.bz2)

  system($${BZIP2} -fdk $${PWD}/fftw3_3.2.2.orig.tar.bz2)
  system($${TAR} -x -f fftw3_3.2.2.orig.tar $${FFTW_HEADERS})
  system(del /S /Q include > NUL 2>&1)
  system(rmdir /S /Q include > NUL 2>&1)
  system(move $${FFTW_VERSION}\\api include > NUL 2>&1)
  system(del /S /Q $${FFTW_VERSION} > NUL 2>&1)
  system(rmdir /S /Q $${FFTW_VERSION} > NUL 2>&1)
  system(del /S /Q fftw3_3.2.2.orig.tar > NUL 2>&1)
}
