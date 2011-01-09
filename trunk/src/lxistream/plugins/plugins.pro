TEMPLATE = subdirs
DESTDIR = .

SUBDIRS += dvdread ffmpeg fftw gui #opengl

linux-g++ {
  SUBDIRS += alsa pulseaudio v4l #linuxdvb
}

win32 {
  #SUBDIRS += winmm
}
