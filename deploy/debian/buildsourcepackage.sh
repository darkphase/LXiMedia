#!/bin/sh

OUTDIR=`pwd`/../../bin
PKGNAME=lximedia-`cat ../../VERSION`
PKGFILE=lximedia_`cat ../../VERSION`-`cat PACKAGE`

rm -f ${OUTDIR}/${PKGFILE}.*
rm -rf ${OUTDIR}/${PKGNAME}

# Build source tree
mkdir -p /${OUTDIR}/${PKGNAME}
cp ../../COPYING /${OUTDIR}/${PKGNAME}/
cp ../../Doxyfile /${OUTDIR}/${PKGNAME}/
cp ../../lximedia.pro /${OUTDIR}/${PKGNAME}/
cp ../../README /${OUTDIR}/${PKGNAME}/
cp ../../VERSION /${OUTDIR}/${PKGNAME}/
mkdir -p /${OUTDIR}/${PKGNAME}/deploy
cp -r ../../deploy/* /${OUTDIR}/${PKGNAME}/deploy/
mkdir -p /${OUTDIR}/${PKGNAME}/ext
cp -r ../../ext/* /${OUTDIR}/${PKGNAME}/ext/
mkdir -p /${OUTDIR}/${PKGNAME}/include
cp -r ../../include/* /${OUTDIR}/${PKGNAME}/include/
mkdir -p /${OUTDIR}/${PKGNAME}/src
cp -r ../../src/* /${OUTDIR}/${PKGNAME}/src/
mkdir -p /${OUTDIR}/${PKGNAME}/test
cp -r ../../test/* /${OUTDIR}/${PKGNAME}/test/

mkdir -p /${OUTDIR}/${PKGNAME}/debian
cp * /${OUTDIR}/${PKGNAME}/debian/
rm /${OUTDIR}/${PKGNAME}/debian/buildsourcepackage.sh

# Create files for allinone package
cp lximediacenter1.init /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.init
cp lximediacenter1.postinst /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.postinst
cp lximediacenter1.postrm /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.postrm

cat liblxiserver1.dirs > /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.dirs
cat liblxistream1.dirs > /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.dirs
cat liblxistreamgui1.dirs >> /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.dirs
cat liblximediacenter1.dirs >> /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.dirs
cat lximediacenter1.dirs >> /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.dirs

cat liblxiserver1.install > /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.install
cat liblxistream1.install > /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.install
cat liblxistreamgui1.install >> /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.install
cat liblximediacenter1.install >> /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.install
cat lximediacenter1.install >> /${OUTDIR}/${PKGNAME}/debian/lximediacenter1-allinone.install

echo "lximedia (`cat ../../VERSION`-`cat PACKAGE`) unstable; urgency=low" > /${OUTDIR}/${PKGNAME}/debian/changelog
echo >> /${OUTDIR}/${PKGNAME}/debian/changelog
echo "  * No changelog" >> /${OUTDIR}/${PKGNAME}/debian/changelog
echo >> /${OUTDIR}/${PKGNAME}/debian/changelog
echo " -- A.J. Admiraal <code@admiraal.dds.nl>  `date -R`" >> /${OUTDIR}/${PKGNAME}/debian/changelog

cd /${OUTDIR}/${PKGNAME} || exit

# Remove makefiles (should be generated with qmake)
rm -rf `find -name Makefile*`

# Remove .svn entries
rm -rf `find -name .svn`

dpkg-buildpackage -rfakeroot -S

cd ..
rm -rf ${PKGNAME}