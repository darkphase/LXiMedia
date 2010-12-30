TEMPLATE = subdirs

FFTW_VERSION = fftw-3.2.2
FFTW_HEADERS = $${FFTW_VERSION}/api/fftw3.h

win32 {
  system(bzip2 -fdk bin.win32\\libfftw3f.a.bz2)
  system(bzip2 -fcdk fftw3_3.2.2.orig.tar.bz2 | tar -x $${FFTW_HEADERS} && rm -rf include && mv $${FFTW_VERSION}/api include && rm -rf $${FFTW_VERSION})
}
