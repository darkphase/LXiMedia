#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"
OUTDIR=${CURDIR}/../../..
PKGFILE=lximedia_`cat ${CURDIR}/../../VERSION`
PKGNAME=lximedia-`cat ${CURDIR}/../../VERSION`

rm -rf /tmp/${PKGNAME}
tar -xzf ${OUTDIR}/${PKGFILE}.tar.gz -C /tmp
cd /tmp/${PKGNAME}/
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=${CURDIR}/Toolchain-i686-w64-mingw32.cmake || exit 1
make || exit 1
makensis -NOCD ${CURDIR}/lximediacenter.nsi || exit 1
cp LXiMediaCenterSetup.exe ${OUTDIR}/LXiMediaCenterSetup-`cat ${CURDIR}/../../VERSION`.exe
rm -rf /tmp/${PKGNAME}

