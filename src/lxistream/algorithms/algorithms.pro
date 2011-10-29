TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt
LXIMEDIA_DIR = ../../..
DESTDIR = $${OUT_PWD}/$${LXIMEDIA_DIR}/bin
TARGET = algorithms
include($${PWD}/$${LXIMEDIA_DIR}/include/config.pri)

# LXiVecIntrin
HEADERS += $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/platform.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrin.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinbool.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinfloat.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinfloat_compare.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinfloat_math.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinfloat_repack.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_compare.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_logic.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_math.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_repack8.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_repack16.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_repack32.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_repack64.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_repackfloat.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/intrinint_set.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/vecbool.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/vecfloat.h \
    $${PWD}/$${LXIMEDIA_DIR}/include/lxivecintrin/vecint.h

# Files
HEADERS += audioconvert.h \
    audioprocess.h \
    data.h \
    deinterlace.h \
    videoconvert.h
SOURCES += audioconvert.cpp \
    audioprocess.cpp \
    data.cpp \
    deinterlace.cpp \
    videoconvert.cpp

# Platform specific
unix|win32-g++ {
  QMAKE_CXXFLAGS_RELEASE -= -O2
  QMAKE_CXXFLAGS += -O3 -funsafe-math-optimizations -funsafe-loop-optimizations
}

# Windows specific
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  TEMPLATE = vclib
  GUID = a9c1840a-e75a-11e0-b8c7-ebaab0e748cb
  DEFINES += _CRT_SECURE_NO_WARNINGS
}
