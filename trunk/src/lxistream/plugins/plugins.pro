TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = .

SUBDIRS += dvdnav ffmpeg freetype gui #opengl
!macx:SUBDIRS += smbclient

unix {
  contains(QMAKE_HOST.os, Linux) {
    SUBDIRS += alsa pulseaudio v4l x11capture #linuxdvb
  }
}

win32 {
  SUBDIRS += gdicapture winmm
}
