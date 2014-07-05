#!/bin/sh
PKGNAME=LXiMediaCenter
VERSION=`cat VERSION`

rm -f ${PKGNAME}_${VERSION}.dmg
hdiutil create -format UDBZ -srcfolder ${PKGNAME}.app ${PKGNAME}_${VERSION}.dmg
