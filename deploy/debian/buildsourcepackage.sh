#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"
OUTDIR=${CURDIR}/../../..
PKGNAME=lximedia-`cat ${CURDIR}/../../VERSION`

rm -rf /tmp/lximedia/${PKGNAME}
mkdir -p /tmp/lximedia/${PKGNAME}
cp ${CURDIR}/../../COPYING /tmp/lximedia/${PKGNAME}/
cp ${CURDIR}/../../CMakeLists.txt /tmp/lximedia/${PKGNAME}/
cp ${CURDIR}/../../README /tmp/lximedia/${PKGNAME}/
cp ${CURDIR}/../../VERSION /tmp/lximedia/${PKGNAME}/
mkdir -p /tmp/lximedia/${PKGNAME}/deploy
cp -r ${CURDIR}/../../deploy/* /tmp/lximedia/${PKGNAME}/deploy/
mkdir -p /tmp/lximedia/${PKGNAME}/ext
cp -r ${CURDIR}/../../ext/* /tmp/lximedia/${PKGNAME}/ext/
mkdir -p /tmp/lximedia/${PKGNAME}/src
cp -r ${CURDIR}/../../src/* /tmp/lximedia/${PKGNAME}/src/
mkdir -p /tmp/lximedia/${PKGNAME}/test
cp -r ${CURDIR}/../../test/* /tmp/lximedia/${PKGNAME}/test/

mkdir -p /tmp/lximedia/${PKGNAME}/debian
cp ${CURDIR}/* /tmp/lximedia/${PKGNAME}/debian/
rm /tmp/lximedia/${PKGNAME}/debian/buildsourcepackage.sh

echo "lximedia (`cat ${CURDIR}/../../VERSION`) unstable; urgency=low" > /tmp/lximedia/${PKGNAME}/debian/changelog
echo >> /tmp/lximedia/${PKGNAME}/debian/changelog
echo "  * No changelog" >> /tmp/lximedia/${PKGNAME}/debian/changelog
echo >> /tmp/lximedia/${PKGNAME}/debian/changelog
echo " -- A.J. Admiraal <code@admiraal.dds.nl>  `date -R`" >> /tmp/lximedia/${PKGNAME}/debian/changelog

cd /tmp/lximedia/${PKGNAME} || exit 1

rm -rf `find -name .svn`

dpkg-buildpackage -rfakeroot -S

cd ..
rm -rf /tmp/lximedia/${PKGNAME}
mv /tmp/lximedia/* ${OUTDIR}/
cd ..
rmdir /tmp/lximedia

