TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = .

# Generate _version.h
unix {
  VERSION = $$system(cat VERSION)
  system(echo \\\"$${VERSION}\\\" > include/_version.h)
}
win32 {
  VERSION = $$system(type VERSION)
  system(echo \"$${VERSION}\" > include\\_version.h)
}

SUBDIRS += ext src test deploy
