TEMPLATE = subdirs
CONFIG += ordered

macx|win32 {
  SUBDIRS = dcraw dvdnav exif ffmpeg freetype
}
