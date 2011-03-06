TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = .

# Generate _version.h
unix|win32-g++:VERSION = $$system(cat VERSION)
unix:system(echo \\\"$${VERSION}\\\" > include/_version.h)
win32:system(echo \"$${VERSION}\" > include\\_version.h)

SUBDIRS += ext src test
