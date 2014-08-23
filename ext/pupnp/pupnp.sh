#!/bin/sh

PUPNP_VERSION=$3

if [ ! -f $2/libupnp-$3/upnp/inc/upnp/upnpconfig.h ]; then
    if [ -f $1/libupnp-$3.tar.xz ]; then
        # Extract
        mkdir -p $2
        cp $1/libupnp-$3.tar.xz $2
        cd $2
        tar -xJf libupnp-$3.tar.xz
        cd libupnp-$3

        # Patch
        patch -p0 < $1/fix-binding-port.patch
        patch -p0 < $1/fix-repeat-announcement-spread.patch
        patch -p0 < $1/add-support-cachecontrol-nocache.patch
        patch -p0 < $1/add-support-multiple-interfaces.patch

        patch -p0 < $1/fix-mingw-build.patch
        patch -p0 < $1/fix-windows-xp-compatibility.patch

        cp $1/autoconfig.h autoconfig.h
        cp $1/upnpconfig.h upnp/inc/upnpconfig.h

        # Copy public headers
        mkdir upnp/inc/upnp
        cp upnp/inc/*.h upnp/inc/upnp
    fi
fi

exit
