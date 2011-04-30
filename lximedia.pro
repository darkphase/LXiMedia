TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = .

# Generate _version.h
unix {
  VERSION = $$system(cat $${PWD}/VERSION)
  system(echo \\\"$${VERSION}\\\" > $${PWD}/include/_version.h)
}
win32 {
  VERSION = $$system(type $$replace(PWD,/,\\)\\VERSION)
  system(echo \"$${VERSION}\" > $$replace(PWD,/,\\)\\include\\_version.h)
}

SUBDIRS += ext src test deploy
