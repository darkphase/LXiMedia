#!/bin/sh

QT_PREFIX="/work/build/______________________________PADDING______________________________/lib/"

QT_COMPONENTS="QtConcurrent QtCore QtDBus QtGui QtMultimedia QtMultimediaWidgets QtNetwork QtOpenGL QtPositioning QtPrintSupport QtQml QtQuick QtSensors QtSql QtWebKit QtWebKitWidgets QtWidgets"

mkdir -p $2/Frameworks

for i in $QT_COMPONENTS
do
    cp -f $1/lib/$i.framework/Versions/Current/$i $2/Frameworks/lib$i.5.dylib
done

for i in $QT_COMPONENTS
do
    for j in $QT_COMPONENTS
    do
        install_name_tool -change $j.framework/Versions/Current/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.5.dylib
        install_name_tool -change $j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.5.dylib
        install_name_tool -change $1/lib/$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.5.dylib
        install_name_tool -change $QT_PREFIX$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.5.dylib
    done
done

QT_PLATFORMS="libqcocoa"

mkdir -p $2/PlugIns/platforms/

for i in $QT_PLATFORMS
do
    cp -f $1/plugins/platforms/$i.dylib $2/PlugIns/platforms/$i.dylib

    for j in $QT_COMPONENTS
    do
        install_name_tool -change $j.framework/Versions/Current/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/platforms/$i.dylib
        install_name_tool -change $j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/platforms/$i.dylib
        install_name_tool -change $1/lib/$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/platforms/$i.dylib
        install_name_tool -change $QT_PREFIX$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/platforms/$i.dylib
    done
done

QT_IMAGEFORMATS="libqgif libqjp2 libqjpeg libqtiff libqwebp"

mkdir -p $2/PlugIns/imageformats/

for i in $QT_IMAGEFORMATS
do
    cp -f $1/plugins/imageformats/$i.dylib $2/PlugIns/imageformats/$i.dylib

    for j in $QT_COMPONENTS
    do
        install_name_tool -change $j.framework/Versions/Current/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/imageformats/$i.dylib
        install_name_tool -change $j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/imageformats/$i.dylib
        install_name_tool -change $1/lib/$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/imageformats/$i.dylib
        install_name_tool -change $QT_PREFIX$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/imageformats/$i.dylib
    done
done

LXI_COMPONENTS="LXiCore LXiStream LXiStreamDevice LXiStreamGui LXiMediaCenter"

for i in $LXI_COMPONENTS
do
    for j in $QT_COMPONENTS
    do
        install_name_tool -change $j.framework/Versions/Current/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.dylib
        install_name_tool -change $j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.dylib
        install_name_tool -change $1/lib/$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.dylib
        install_name_tool -change $QT_PREFIX$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/Frameworks/lib$i.dylib
    done

    for j in $LXI_COMPONENTS
    do
        install_name_tool -change lib$j.0.dylib @executable_path/../Frameworks/lib$j.0.dylib $2/Frameworks/lib$i.dylib
    done
done

LXI_PLUGINS="lximediacenter_mediaplayer lxistream_dvdnav lxistream_ffmpeg lxistream_freetype lxistream_gui"

for i in $LXI_PLUGINS
do
    for j in $QT_COMPONENTS
    do
        install_name_tool -change $j.framework/Versions/Current/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/lximedia0/lib$i.dylib
        install_name_tool -change $j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/lximedia0/lib$i.dylib
        install_name_tool -change $1/lib/$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/lximedia0/lib$i.dylib
        install_name_tool -change $QT_PREFIX$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/PlugIns/lximedia0/lib$i.dylib
    done

    for j in $LXI_COMPONENTS
    do
        install_name_tool -change lib$j.0.dylib @executable_path/../Frameworks/lib$j.0.dylib $2/PlugIns/lximedia0/lib$i.dylib
    done
done

LXI_EXECUTABLES="lximcbackend LXiMediaCenter"

for i in $LXI_EXECUTABLES
do
    for j in $QT_COMPONENTS
    do
        install_name_tool -change $j.framework/Versions/Current/$j @executable_path/../Frameworks/lib$j.5.dylib $2/MacOS/$i
        install_name_tool -change $j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/MacOS/$i
        install_name_tool -change $1/lib/$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/MacOS/$i
        install_name_tool -change $QT_PREFIX$j.framework/Versions/5/$j @executable_path/../Frameworks/lib$j.5.dylib $2/MacOS/$i
    done

    for j in $LXI_COMPONENTS
    do
        install_name_tool -change lib$j.0.dylib @executable_path/../Frameworks/lib$j.0.dylib $2/MacOS/$i
    done
done
