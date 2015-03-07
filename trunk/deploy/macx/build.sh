#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"
OUTDIR=${CURDIR}/../../../publish
PKGNAME=lximediaserver_`cat ${CURDIR}/../../VERSION`
DMGFILE=${OUTDIR}/${PKGNAME}.dmg

rm -rf /tmp/lximediaserver/${PKGNAME}
mkdir -p /tmp/lximediaserver/${PKGNAME}
cp -r ${CURDIR}/../../* /tmp/lximediaserver/${PKGNAME}

cd /tmp/lximediaserver/${PKGNAME} || exit 1

cmake -DCMAKE_BUILD_TYPE=Release || exit 1
make || exit 1
./LXiMediaServer.app/Contents/MacOS/lximediaserver --probeplugins 2> /dev/null

mkdir -p ${OUTDIR}/
rm -f ${DMGFILE}
hdiutil create -format UDBZ -srcfolder LXiMediaServer.app ${DMGFILE} || exit 1
rm -rf /tmp/lximediaserver
