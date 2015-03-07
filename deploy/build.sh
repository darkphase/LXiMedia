#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"

UNAME=`uname`
if [ "${UNAME}" = 'Linux' ]; then
    sh ${CURDIR}/debian/buildsourcepackage.sh || exit 1
    sh ${CURDIR}/debian/build.sh || exit 1
    sh ${CURDIR}/win32/build.sh || exit 1
elif [ "${UNAME}" = 'Darwin' ]; then
    sh ${CURDIR}/macx/build.sh || exit 1
fi
