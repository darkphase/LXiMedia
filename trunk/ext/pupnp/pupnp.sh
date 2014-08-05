#!/bin/sh

PUPNP_VERSION=$3

if [ ! -f $2/libupnp-$3/Makefile ]; then
    if [ -f $1/libupnp-$3.tar.bz2 ]; then
        # Extract
        mkdir -p $2
        cp $1/libupnp-$3.tar.bz2 $2
        cd $2
        tar -xjf libupnp-$3.tar.bz2
        cd libupnp-$3

        # Patch
        patch -p0 < $1/fix-binding-port.patch
        patch -p0 < $1/fix-repeat-announcement-spread.patch
        patch -p0 < $1/add-support-cachecontrol-nocache.patch
        patch -p0 < $1/add-support-multiple-interfaces.patch

        #patch -p0 < $1/fix-mingw47-build.patch
        #patch -p0 < $1/fix-windows-xp-compatibility.patch

        # Configure
        sh configure --enable-static --disable-shared --disable-samples --disable-dependency-tracking CFLAGS="-w -fPIC"

        # Make
        make

        # Copy public headers
        mkdir upnp/inc/upnp
        cp upnp/inc/*.h upnp/inc/upnp
    fi
fi

exit
