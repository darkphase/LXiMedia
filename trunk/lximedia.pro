TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = .

# Generate _version.h
unix {
  VERSION = $$system(cat $${PWD}/VERSION)
  system(echo // Generated from lximedia.pro     >  $${PWD}/include/_version.h)
  system(echo \\\"$${VERSION}\\\"                >> $${PWD}/include/_version.h)
}
win32 {
  VERSION = $$system(type $$replace(PWD,/,\\)\\VERSION)
  system(echo // Generated from lximedia.pro     >  $$replace(PWD,/,\\)\\include\\_version.h)
  system(echo \"$${VERSION}\"                    >> $$replace(PWD,/,\\)\\include\\_version.h)
}

# Generate stdint.h and inttypes.h
win32-msvc2005|win32-msvc2008|win32-msvc2010 {
  system(echo // Generated from lximedia.pro     >  $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef signed __int8     int8_t;  >> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef unsigned __int8   uint8_t; >> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef signed __int16    int16_t; >> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef unsigned __int16  uint16_t;>> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef signed __int32    int32_t; >> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef unsigned __int32  uint32_t;>> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef signed __int64    int64_t; >> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef unsigned __int64  uint64_t;>> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(echo typedef off_t             ssize_t; >> $$replace(PWD,/,\\)\\include\\stdint.h)
  system(copy /Y $$replace(PWD,/,\\)\\include\\stdint.h $$replace(PWD,/,\\)\\include\\inttypes.h > NUL)
}

SUBDIRS += ext src test deploy
