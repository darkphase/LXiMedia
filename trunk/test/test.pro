TEMPLATE = subdirs
CONFIG += ordered
DESTDIR = .

SUBDIRS += liblxicoretest liblxiservertest liblximediacentertest

!macx:SUBDIRS += liblxistreamtest lxistreamfiletest
