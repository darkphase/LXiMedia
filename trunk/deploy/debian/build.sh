#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"
OUTDIR=${CURDIR}/../../../publish
PKGFILE=lximediaserver_`cat ${CURDIR}/../../VERSION`

mkdir -p ${CURDIR}/../../../pbuilder

for ARCHITECTURE in i386 amd64
do
    for DISTRIBUTION in trusty utopic
    do
	BASETGZ=${CURDIR}/../../../pbuilder/${DISTRIBUTION}-${ARCHITECTURE}-base.tgz
	if [ ! -f ${BASETGZ} ]; then
	    sudo pbuilder --create --basetgz ${BASETGZ} --distribution ${DISTRIBUTION} --architecture ${ARCHITECTURE} || exit 1
	    sudo pbuilder --update --basetgz ${BASETGZ} --distribution ${DISTRIBUTION} --architecture ${ARCHITECTURE} --components "main universe" --override-config || exit 1
	fi
	if [ -f ${BASETGZ} ]; then
	    sudo rm -f cp /var/cache/pbuilder/result/*
	    sudo pbuilder --build --basetgz ${BASETGZ} --distribution ${DISTRIBUTION} --architecture ${ARCHITECTURE} ${OUTDIR}/${PKGFILE}.dsc || exit 1
	    mkdir -p ${OUTDIR}/${DISTRIBUTION}/
	    cp /var/cache/pbuilder/result/*.deb ${OUTDIR}/${DISTRIBUTION}/
	fi
    done
done

