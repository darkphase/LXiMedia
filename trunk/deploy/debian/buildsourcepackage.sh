#!/bin/sh

OUTDIR=`pwd`/../../..
PKGNAME=lximedia-`cat ../../VERSION`
PKGFILE=lximedia_`cat ../../VERSION`-`cat PACKAGE`

rm -f ${OUTDIR}/${PKGFILE}.*
rm -rf ${OUTDIR}/${PKGNAME}

# Build source tree
mkdir -p /${OUTDIR}/${PKGNAME}
cp ../../COPYING /${OUTDIR}/${PKGNAME}/
cp ../../CMakeLists.txt /${OUTDIR}/${PKGNAME}/
cp ../../README /${OUTDIR}/${PKGNAME}/
cp ../../VERSION /${OUTDIR}/${PKGNAME}/
mkdir -p /${OUTDIR}/${PKGNAME}/deploy
cp -r ../../deploy/* /${OUTDIR}/${PKGNAME}/deploy/
mkdir -p /${OUTDIR}/${PKGNAME}/ext
cp -r ../../ext/* /${OUTDIR}/${PKGNAME}/ext/
mkdir -p /${OUTDIR}/${PKGNAME}/src
cp -r ../../src/* /${OUTDIR}/${PKGNAME}/src/
mkdir -p /${OUTDIR}/${PKGNAME}/test
cp -r ../../test/* /${OUTDIR}/${PKGNAME}/test/

mkdir -p /${OUTDIR}/${PKGNAME}/debian
cp * /${OUTDIR}/${PKGNAME}/debian/
rm /${OUTDIR}/${PKGNAME}/debian/buildsourcepackage.sh

echo "lximedia (`cat ../../VERSION`-`cat PACKAGE`) unstable; urgency=low" > /${OUTDIR}/${PKGNAME}/debian/changelog
echo >> /${OUTDIR}/${PKGNAME}/debian/changelog
echo "  * No changelog" >> /${OUTDIR}/${PKGNAME}/debian/changelog
echo >> /${OUTDIR}/${PKGNAME}/debian/changelog
echo " -- A.J. Admiraal <code@admiraal.dds.nl>  `date -R`" >> /${OUTDIR}/${PKGNAME}/debian/changelog

cd /${OUTDIR}/${PKGNAME} || exit

# Remove .svn entries
rm -rf `find -name .svn`

dpkg-buildpackage -rfakeroot -S

cd ..
rm -rf ${PKGNAME}

# Generate package build script
#echo if [ \$# -ne 1 ] >  ${OUTDIR}/${PKGFILE}.sh
#echo then >> ${OUTDIR}/${PKGFILE}.sh
#echo   echo Usage: sudo \$0 distribution >> ${OUTDIR}/${PKGFILE}.sh
#echo   exit 65 >> ${OUTDIR}/${PKGFILE}.sh
#echo fi >> ${OUTDIR}/${PKGFILE}.sh
#echo >> ${OUTDIR}/${PKGFILE}.sh
#echo grep \"debian\" /etc/issue -i -q>> ${OUTDIR}/${PKGFILE}.sh
#echo if [ \$? = \'0\' ] >> ${OUTDIR}/${PKGFILE}.sh
#echo then >> ${OUTDIR}/${PKGFILE}.sh
#echo   pbuilder --update --distribution \$1 --components \"main\" --override-config \|\| exit 1 >> ${OUTDIR}/${PKGFILE}.sh
#echo fi >> ${OUTDIR}/${PKGFILE}.sh
#echo >> ${OUTDIR}/${PKGFILE}.sh
#echo grep \"ubuntu\" /etc/issue -i -q >> ${OUTDIR}/${PKGFILE}.sh
#echo if [ \$? = \'0\' ] >> ${OUTDIR}/${PKGFILE}.sh
#echo then >> ${OUTDIR}/${PKGFILE}.sh
#echo   pbuilder --update --distribution \$1 --components \"main universe\" --override-config \|\| exit 1 >> ${OUTDIR}/${PKGFILE}.sh
#echo fi >> ${OUTDIR}/${PKGFILE}.sh
#echo >> ${OUTDIR}/${PKGFILE}.sh
echo pbuilder --build --distribution \$1 ${PKGFILE}.dsc \|\| exit 1 >> ${OUTDIR}/${PKGFILE}.sh
chmod a+x ${OUTDIR}/${PKGFILE}.sh
