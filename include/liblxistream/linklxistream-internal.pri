include($${LXIMEDIA_DIR}/include/liblxistream/linklxistream.pri)

linux-g++|win32-g++ {
  QMAKE_CXXFLAGS += -include $${LXIMEDIA_DIR}/obj/LXiStream/LXiStream
# -Winvalid-pch
}
