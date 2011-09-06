TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = .

SUBDIRS += dvdnav ffmpeg fftw gui #mms #opengl

unix {
  contains(QMAKE_HOST.os, Linux) {
    SUBDIRS += alsa pulseaudio v4l #linuxdvb
  }
}

win32 {
  #SUBDIRS += winmm
}
