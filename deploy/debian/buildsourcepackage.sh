#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"
OUTDIR=${CURDIR}/../../..
PKGNAME=lximediaserver-`cat ${CURDIR}/../../VERSION`

rm -rf /tmp/lximediaserver/${PKGNAME}
mkdir -p /tmp/lximediaserver/${PKGNAME}
cp -r ${CURDIR}/../../* /tmp/lximediaserver/${PKGNAME}

echo "lximediaserver (`cat ${CURDIR}/../../VERSION`) unstable; urgency=low" > /tmp/lximediaserver/${PKGNAME}/debian/changelog
echo >> /tmp/lximediaserver/${PKGNAME}/debian/changelog
echo "  * No changelog" >> /tmp/lximediaserver/${PKGNAME}/debian/changelog
echo >> /tmp/lximediaserver/${PKGNAME}/debian/changelog
echo " -- A.J. Admiraal <code@admiraal.dds.nl>  `date -R`" >> /tmp/lximediaserver/${PKGNAME}/debian/changelog

cd /tmp/lximediaserver/${PKGNAME} || exit 1

rm -rf `find -name .svn`

dpkg-buildpackage -rfakeroot -S

cd ..
rm -rf /tmp/lximediaserver/${PKGNAME}
mv /tmp/lximediaserver/* ${OUTDIR}/
cd ..
rmdir /tmp/lximediaserver

