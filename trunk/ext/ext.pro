TEMPLATE = subdirs
CONFIG += ordered

macx|win32 {
  SUBDIRS = dvdnav exif ffmpeg fftw
}
