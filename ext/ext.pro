TEMPLATE = subdirs
CONFIG += ordered

macx|win32 {
  SUBDIRS = dcraw dvdnav exif freetype
}

unix|win32 {
  SUBDIRS += pupnp ffmpeg
}
