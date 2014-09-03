#!/bin/sh

CURDIR="$( cd "$( dirname "$0" )" && pwd )"

sh ${CURDIR}/debian/buildsourcepackage.sh || exit 1
sh ${CURDIR}/debian/build.sh || exit 1
sh ${CURDIR}/win32/build.sh || exit 1

