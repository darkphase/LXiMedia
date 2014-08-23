#!/bin/sh

if [ ! -f $2/vlc-$3/include/vlc/vlc.h ]; then
    if [ -f $1/vlc-$3.tar.xz ]; then
        # Extract
        mkdir -p $2
        cp $1/vlc-$3.tar.xz $2
        cd $2
        tar -xJf vlc-$3.tar.xz
    fi
fi

exit
