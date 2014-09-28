#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"
OUTDIR=${CURDIR}/../../..
PKGFILE=lximediaserver_`cat ${CURDIR}/../../VERSION`
PKGNAME=lximediaserver-`cat ${CURDIR}/../../VERSION`

rm -rf /tmp/${PKGNAME}
tar -xzf ${OUTDIR}/${PKGFILE}.tar.gz -C /tmp
cd /tmp/${PKGNAME}/
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${CURDIR}/Toolchain-i686-w64-mingw32.cmake || exit 1
make || exit 1
makensis -NOCD ${CURDIR}/lximediaserver.nsi || exit 1
cp LXiMediaServerSetup.exe ${OUTDIR}/LXiMediaServerSetup-`cat ${CURDIR}/../../VERSION`.exe
rm -rf /tmp/${PKGNAME}

