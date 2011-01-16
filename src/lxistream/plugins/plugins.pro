TEMPLATE = subdirs
DESTDIR = .

SUBDIRS += dvdnav ffmpeg fftw gui #opengl

linux-g++ {
  SUBDIRS += alsa pulseaudio v4l #linuxdvb
}

win32 {
  #SUBDIRS += winmm
}
