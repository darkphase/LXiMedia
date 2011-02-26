include($${LXIMEDIA_DIR}/include/liblximediacenter/linklximediacenter.pri)

linux-g++|win32-g++ {
  QMAKE_CXXFLAGS += -include $${LXIMEDIA_DIR}/obj/LXiMediaCenter/LXiMediaCenter
# -Winvalid-pch
}
